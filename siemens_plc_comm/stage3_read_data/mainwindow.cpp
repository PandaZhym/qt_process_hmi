#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFont>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();

    // Worker 线程（必须先创建，再 setupConnections，否则野指针崩溃）
    m_thread = new QThread(this);
    m_worker = new PlcWorker();
    m_worker->moveToThread(m_thread);

    setupConnections();  // 必须在 m_worker 创建之后调用

    m_thread->start();
}

MainWindow::~MainWindow()
{
    m_thread->quit();
    m_thread->wait();
}

// ==================== UI 搭建 ====================
void MainWindow::setupUi()
{
    setWindowTitle("S7-1200 数据读取");
    resize(520, 420);

    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    // ---------- 第一块：连接 ----------
    QGroupBox *connGroup = new QGroupBox("连接设置");
    QVBoxLayout *connLayout = new QVBoxLayout(connGroup);

    QHBoxLayout *ipRow = new QHBoxLayout();
    m_ipEdit = new QLineEdit("192.168.0.1");
    m_connectBtn = new QPushButton("连接 PLC");
    m_disconnectBtn = new QPushButton("断开");
    m_disconnectBtn->setEnabled(false);
    ipRow->addWidget(new QLabel("PLC IP:"));
    ipRow->addWidget(m_ipEdit);
    ipRow->addWidget(m_connectBtn);
    ipRow->addWidget(m_disconnectBtn);
    connLayout->addLayout(ipRow);

    m_statusLabel = new QLabel("● 未连接");
    QFont statusFont = m_statusLabel->font();
    statusFont.setPointSize(14);
    m_statusLabel->setFont(statusFont);
    connLayout->addWidget(m_statusLabel);

    mainLayout->addWidget(connGroup);

    // ---------- 第二块：读取 ----------
    QGroupBox *readGroup = new QGroupBox("读取数据");
    QVBoxLayout *readLayout = new QVBoxLayout(readGroup);

    // 参数行
    QHBoxLayout *paramRow = new QHBoxLayout();
    m_areaCombo = new QComboBox();
    m_areaCombo->addItem("M 区（中间继电器）");
    m_areaCombo->addItem("I 区（输入映像）");
    m_areaCombo->addItem("Q 区（输出映像）");
    m_areaCombo->addItem("DB 块（数据块）");

    m_dbLabel = new QLabel("DB 号:");
    m_dbSpin = new QSpinBox();
    m_dbSpin->setRange(1, 65535);
    m_dbSpin->setValue(1);
    // 默认选 M 区，隐藏 DB 号输入
    m_dbLabel->setVisible(false);
    m_dbSpin->setVisible(false);

    paramRow->addWidget(new QLabel("区域:"));
    paramRow->addWidget(m_areaCombo);
    paramRow->addWidget(m_dbLabel);
    paramRow->addWidget(m_dbSpin);
    paramRow->addWidget(new QLabel("起始地址:"));
    m_startSpin = new QSpinBox();
    m_startSpin->setRange(0, 65535);
    paramRow->addWidget(m_startSpin);
    paramRow->addWidget(new QLabel("字节数:"));
    m_sizeSpin = new QSpinBox();
    m_sizeSpin->setRange(1, 256);
    m_sizeSpin->setValue(10);
    paramRow->addWidget(m_sizeSpin);
    readLayout->addLayout(paramRow);

    // 读取按钮 + 解析类型
    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addWidget(new QLabel("解析为:"));
    m_typeCombo = new QComboBox();
    m_typeCombo->addItem("原始 (HEX)",       -1);
    m_typeCombo->addItem("BOOL (位)",         0);
    m_typeCombo->addItem("INT8 (有符号)",     1);
    m_typeCombo->addItem("UINT8 (无符号)",    2);
    m_typeCombo->addItem("INT16 (有符号)",    3);
    m_typeCombo->addItem("UINT16 (无符号)",   4);
    m_typeCombo->addItem("INT32 (有符号)",    5);
    m_typeCombo->addItem("UINT32 (无符号)",   6);
    m_typeCombo->addItem("FLOAT32 (浮点)",    7);
    m_typeCombo->setCurrentIndex(4);  // 默认 INT16
    btnRow->addWidget(m_typeCombo);
    btnRow->addStretch();
    m_readBtn = new QPushButton("读取");
    m_readBtn->setEnabled(false);
    btnRow->addWidget(m_readBtn);
    readLayout->addLayout(btnRow);

    // 结果显示
    QFont monoFont("Consolas", 11);
    readLayout->addWidget(new QLabel("原始数据 (HEX):"));
    m_hexLabel = new QLabel("—");
    m_hexLabel->setFont(monoFont);
    m_hexLabel->setWordWrap(true);
    readLayout->addWidget(m_hexLabel);

    readLayout->addWidget(new QLabel("解析值:"));
    m_valueLabel = new QLabel("—");
    QFont valFont("Consolas", 12);
    valFont.setBold(true);
    m_valueLabel->setFont(valFont);
    readLayout->addWidget(m_valueLabel);

    mainLayout->addWidget(readGroup);
}

// ==================== 信号/槽 连接 ====================
void MainWindow::setupConnections()
{
    // 按钮点击
    connect(m_connectBtn, &QPushButton::clicked,
            this, &MainWindow::onConnectClicked);
    connect(m_disconnectBtn, &QPushButton::clicked,
            this, &MainWindow::onDisconnectClicked);
    connect(m_readBtn, &QPushButton::clicked,
            this, &MainWindow::onReadClicked);

    // 区域切换 → 显示/隐藏 DB 号
    connect(m_areaCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onAreaChanged);

    // UI → Worker（跨线程）
    connect(this, &MainWindow::requestConnect,
            m_worker, &PlcWorker::connectToPlc);
    connect(this, &MainWindow::requestDisconnect,
            m_worker, &PlcWorker::disconnectFromPlc);
    connect(this, &MainWindow::requestRead,
            m_worker, &PlcWorker::readArea);

    // Worker → UI（跨线程）
    connect(m_worker, &PlcWorker::connected,
            this, &MainWindow::onConnected);
    connect(m_worker, &PlcWorker::disconnected,
            this, &MainWindow::onDisconnected);
    connect(m_worker, &PlcWorker::errorOccurred,
            this, &MainWindow::onError);
    connect(m_worker, &PlcWorker::dataRead,
            this, &MainWindow::onDataRead);

    // 线程退出 → 清理 worker
    connect(m_thread, &QThread::finished,
            m_worker, &QObject::deleteLater);
}

// ==================== 连接相关 ====================
void MainWindow::onConnectClicked()
{
    m_statusLabel->setText("● 正在连接...");
    m_connectBtn->setEnabled(false);
    emit requestConnect(m_ipEdit->text(), 0, 1);
}

void MainWindow::onDisconnectClicked()
{
    emit requestDisconnect();
}

void MainWindow::onConnected()
{
    m_statusLabel->setStyleSheet("color: green;");
    m_statusLabel->setText("● 已连接");
    m_connectBtn->setEnabled(false);
    m_disconnectBtn->setEnabled(true);
    m_readBtn->setEnabled(true);
}

void MainWindow::onDisconnected()
{
    m_statusLabel->setStyleSheet("color: gray;");
    m_statusLabel->setText("● 未连接");
    m_connectBtn->setEnabled(true);
    m_disconnectBtn->setEnabled(false);
    m_readBtn->setEnabled(false);
}

void MainWindow::onError(int code, const QString &message)
{
    m_statusLabel->setStyleSheet("color: red;");
    m_statusLabel->setText(QString("● 错误 %1: %2").arg(code).arg(message));
    m_connectBtn->setEnabled(true);
    m_disconnectBtn->setEnabled(false);
    m_readBtn->setEnabled(false);
}

// ==================== 读取相关 ====================
void MainWindow::onReadClicked()
{
    m_hexLabel->setText("读取中...");
    m_valueLabel->setText("—");

    int area   = m_areaCombo->currentIndex();  // 0=M,1=I,2=Q,3=DB
    int dbNum  = m_dbSpin->value();
    int start  = m_startSpin->value();
    int size   = m_sizeSpin->value();

    emit requestRead(area, dbNum, start, size);
}

void MainWindow::onAreaChanged(int index)
{
    // 只有选 DB 区时才显示 DB 号输入
    bool isDB = (index == AREA_DB);
    m_dbLabel->setVisible(isDB);
    m_dbSpin->setVisible(isDB);
}

void MainWindow::onDataRead(int areaType, int dbNumber, int start,
                            const QByteArray &data)
{
    // 显示原始 HEX
    m_hexLabel->setText(formatHex(data));

    // 解析值
    int valueType = m_typeCombo->currentData().toInt();
    m_valueLabel->setText(parseValue(data, valueType));
}

// ==================== 工具函数 ====================
QString MainWindow::formatHex(const QByteArray &data) const
{
    QString result;
    for (int i = 0; i < data.size(); ++i) {
        if (i > 0 && i % 16 == 0)
            result += "\n";                     // 每 16 字节换行
        result += QString("%1 ")
                      .arg((unsigned char)data.at(i), 2, 16, QChar('0'))
                      .toUpper();
    }
    return result.trimmed();
}

QString MainWindow::parseValue(const QByteArray &data, int valueType) const
{
    if (data.isEmpty())
        return "（无数据）";

    auto getU8 = [&](int off) -> quint8 {
        return (quint8)data.at(off);
    };
    auto getS8 = [&](int off) -> qint8 {
        return (qint8)data.at(off);
    };
    // S7-1200 使用大端字节序（Big Endian）
    auto getU16BE = [&](int off) -> quint16 {
        return ((quint8)data.at(off) << 8) | (quint8)data.at(off + 1);
    };
    auto getS16BE = [&](int off) -> qint16 {
        return (qint16)getU16BE(off);
    };
    auto getU32BE = [&](int off) -> quint32 {
        return ((quint32)getU16BE(off) << 16) | getU16BE(off + 2);
    };
    auto getS32BE = [&](int off) -> qint32 {
        return (qint32)getU32BE(off);
    };
    auto getFloatBE = [&](int off) -> float {
        quint32 bits = getU32BE(off);
        float f;
        memcpy(&f, &bits, 4);
        return f;
    };

    switch (valueType) {
    case -1:  // 原始 HEX（已在上方显示）
        return QString("共 %1 字节").arg(data.size());
    case 0: { // BOOL 位
        QStringList bits;
        for (int i = 0; i < data.size(); ++i) {
            quint8 byte = getU8(i);
            for (int b = 0; b < 8; ++b)
                bits << ((byte >> b) & 1 ? "1" : "0");
        }
        return bits.join(' ');
    }
    case 1: { // INT8
        QStringList vals;
        for (int i = 0; i < data.size(); ++i)
            vals << QString::number(getS8(i));
        return vals.join(", ");
    }
    case 2: { // UINT8
        QStringList vals;
        for (int i = 0; i < data.size(); ++i)
            vals << QString::number(getU8(i));
        return vals.join(", ");
    }
    case 3: { // INT16 (大端)
        QStringList vals;
        for (int i = 0; i + 1 < data.size(); i += 2)
            vals << QString::number(getS16BE(i));
        return vals.join(", ");
    }
    case 4: { // UINT16 (大端)
        QStringList vals;
        for (int i = 0; i + 1 < data.size(); i += 2)
            vals << QString::number(getU16BE(i));
        return vals.join(", ");
    }
    case 5: { // INT32 (大端)
        QStringList vals;
        for (int i = 0; i + 3 < data.size(); i += 4)
            vals << QString::number(getS32BE(i));
        return vals.join(", ");
    }
    case 6: { // UINT32 (大端)
        QStringList vals;
        for (int i = 0; i + 3 < data.size(); i += 4)
            vals << QString::number(getU32BE(i));
        return vals.join(", ");
    }
    case 7: { // FLOAT32 (大端)
        QStringList vals;
        for (int i = 0; i + 3 < data.size(); i += 4)
            vals << QString::number(getFloatBE(i), 'f', 3);
        return vals.join(", ");
    }
    default:
        return "未知类型";
    }
}
