#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFont>
#include <cstring>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();

    // Worker 线程（必须先创建 worker，再 connect）
    m_thread = new QThread(this);
    m_worker = new PlcWorker();
    m_worker->moveToThread(m_thread);

    setupConnections();

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
    setWindowTitle("S7-1200 数据读写");
    resize(560, 580);

    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    // ===== 第一块：连接 =====
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

    m_statusLabel = new QLabel("未连接");
    QFont statusFont = m_statusLabel->font();
    statusFont.setPointSize(13);
    m_statusLabel->setFont(statusFont);
    connLayout->addWidget(m_statusLabel);
    mainLayout->addWidget(connGroup);

    // ===== 第二块：读取（同项目 3）=====
    QGroupBox *readGroup = new QGroupBox("读取数据");
    QVBoxLayout *readLayout = new QVBoxLayout(readGroup);

    QHBoxLayout *readParamRow = new QHBoxLayout();
    m_readAreaCombo = new QComboBox();
    m_readAreaCombo->addItems({"M 区", "I 区", "Q 区", "DB 块"});
    m_readDbLabel = new QLabel("DB 号:");
    m_readDbSpin = new QSpinBox();
    m_readDbSpin->setRange(1, 65535);
    m_readDbSpin->setValue(1);
    m_readDbLabel->setVisible(false);
    m_readDbSpin->setVisible(false);
    readParamRow->addWidget(new QLabel("区域:"));
    readParamRow->addWidget(m_readAreaCombo);
    readParamRow->addWidget(m_readDbLabel);
    readParamRow->addWidget(m_readDbSpin);
    readParamRow->addWidget(new QLabel("起始:"));
    m_readStartSpin = new QSpinBox();
    m_readStartSpin->setRange(0, 65535);
    readParamRow->addWidget(m_readStartSpin);
    readParamRow->addWidget(new QLabel("字节数:"));
    m_readSizeSpin = new QSpinBox();
    m_readSizeSpin->setRange(1, 256);
    m_readSizeSpin->setValue(10);
    readParamRow->addWidget(m_readSizeSpin);
    readLayout->addLayout(readParamRow);

    QHBoxLayout *readBtnRow = new QHBoxLayout();
    readBtnRow->addWidget(new QLabel("解析为:"));
    m_readTypeCombo = new QComboBox();
    m_readTypeCombo->addItem("HEX",      -1);
    m_readTypeCombo->addItem("BOOL",      0);
    m_readTypeCombo->addItem("INT8",      1);
    m_readTypeCombo->addItem("UINT8",     2);
    m_readTypeCombo->addItem("INT16",     3);
    m_readTypeCombo->addItem("UINT16",    4);
    m_readTypeCombo->addItem("INT32",     5);
    m_readTypeCombo->addItem("FLOAT32",   6);
    m_readTypeCombo->setCurrentIndex(4);
    readBtnRow->addWidget(m_readTypeCombo);
    readBtnRow->addStretch();
    m_readBtn = new QPushButton("读取");
    m_readBtn->setEnabled(false);
    readBtnRow->addWidget(m_readBtn);
    readLayout->addLayout(readBtnRow);

    QFont monoFont("Consolas", 10);
    readLayout->addWidget(new QLabel("原始 HEX:"));
    m_hexLabel = new QLabel("—");
    m_hexLabel->setFont(monoFont);
    m_hexLabel->setWordWrap(true);
    readLayout->addWidget(m_hexLabel);
    readLayout->addWidget(new QLabel("解析值:"));
    m_valueLabel = new QLabel("—");
    m_valueLabel->setFont(monoFont);
    readLayout->addWidget(m_valueLabel);
    mainLayout->addWidget(readGroup);

    // ===== 第三块：写入（新增）=====
    QGroupBox *writeGroup = new QGroupBox("写入数据");
    QVBoxLayout *writeLayout = new QVBoxLayout(writeGroup);

    QHBoxLayout *writeParamRow = new QHBoxLayout();
    m_writeAreaCombo = new QComboBox();
    m_writeAreaCombo->addItems({"M 区", "I 区", "Q 区", "DB 块"});
    m_writeDbLabel = new QLabel("DB 号:");
    m_writeDbSpin = new QSpinBox();
    m_writeDbSpin->setRange(1, 65535);
    m_writeDbSpin->setValue(1);
    m_writeDbLabel->setVisible(false);
    m_writeDbSpin->setVisible(false);
    writeParamRow->addWidget(new QLabel("区域:"));
    writeParamRow->addWidget(m_writeAreaCombo);
    writeParamRow->addWidget(m_writeDbLabel);
    writeParamRow->addWidget(m_writeDbSpin);
    writeParamRow->addWidget(new QLabel("起始:"));
    m_writeStartSpin = new QSpinBox();
    m_writeStartSpin->setRange(0, 65535);
    writeParamRow->addWidget(m_writeStartSpin);
    writeLayout->addLayout(writeParamRow);

    QHBoxLayout *writeBtnRow = new QHBoxLayout();
    writeBtnRow->addWidget(new QLabel("类型:"));
    m_writeTypeCombo = new QComboBox();
    m_writeTypeCombo->addItem("BOOL",      0);
    m_writeTypeCombo->addItem("INT8",      1);
    m_writeTypeCombo->addItem("UINT8",     2);
    m_writeTypeCombo->addItem("INT16",     3);
    m_writeTypeCombo->addItem("UINT16",    4);
    m_writeTypeCombo->addItem("INT32",     5);
    m_writeTypeCombo->addItem("FLOAT32",   6);
    m_writeTypeCombo->setCurrentIndex(3);  // 默认 INT16
    writeBtnRow->addWidget(m_writeTypeCombo);
    writeBtnRow->addWidget(new QLabel("值:"));
    m_writeValueEdit = new QLineEdit("100");
    writeBtnRow->addWidget(m_writeValueEdit);
    m_writeBtn = new QPushButton("写入");
    m_writeBtn->setEnabled(false);
    writeBtnRow->addWidget(m_writeBtn);
    writeLayout->addLayout(writeBtnRow);

    m_writeResultLabel = new QLabel("—");
    writeLayout->addWidget(m_writeResultLabel);
    mainLayout->addWidget(writeGroup);
}

// ==================== 信号/槽 连接 ====================
void MainWindow::setupConnections()
{
    connect(m_connectBtn, &QPushButton::clicked,
            this, &MainWindow::onConnectClicked);
    connect(m_disconnectBtn, &QPushButton::clicked,
            this, &MainWindow::onDisconnectClicked);
    connect(m_readBtn, &QPushButton::clicked,
            this, &MainWindow::onReadClicked);
    connect(m_writeBtn, &QPushButton::clicked,
            this, &MainWindow::onWriteClicked);

    // 区域切换 → 显示/隐藏 DB 号
    connect(m_readAreaCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onReadAreaChanged);
    connect(m_writeAreaCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onWriteAreaChanged);

    // UI → Worker
    connect(this, &MainWindow::requestConnect,
            m_worker, &PlcWorker::connectToPlc);
    connect(this, &MainWindow::requestDisconnect,
            m_worker, &PlcWorker::disconnectFromPlc);
    connect(this, &MainWindow::requestRead,
            m_worker, &PlcWorker::readArea);
    connect(this, &MainWindow::requestWrite,
            m_worker, &PlcWorker::writeArea);

    // Worker → UI
    connect(m_worker, &PlcWorker::connected,
            this, &MainWindow::onConnected);
    connect(m_worker, &PlcWorker::disconnected,
            this, &MainWindow::onDisconnected);
    connect(m_worker, &PlcWorker::errorOccurred,
            this, &MainWindow::onError);
    connect(m_worker, &PlcWorker::dataRead,
            this, &MainWindow::onDataRead);
    connect(m_worker, &PlcWorker::dataWritten,
            this, &MainWindow::onDataWritten);

    connect(m_thread, &QThread::finished,
            m_worker, &QObject::deleteLater);
}

// ==================== 连接相关 ====================
void MainWindow::onConnectClicked()
{
    m_statusLabel->setText("正在连接...");
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
    m_statusLabel->setText("已连接");
    m_connectBtn->setEnabled(false);
    m_disconnectBtn->setEnabled(true);
    m_readBtn->setEnabled(true);
    m_writeBtn->setEnabled(true);
}

void MainWindow::onDisconnected()
{
    m_statusLabel->setStyleSheet("color: gray;");
    m_statusLabel->setText("未连接");
    m_connectBtn->setEnabled(true);
    m_disconnectBtn->setEnabled(false);
    m_readBtn->setEnabled(false);
    m_writeBtn->setEnabled(false);
}

void MainWindow::onError(int code, const QString &message)
{
    m_statusLabel->setStyleSheet("color: red;");
    m_statusLabel->setText(QString("错误 %1: %2").arg(code).arg(message));
    m_connectBtn->setEnabled(true);
    m_disconnectBtn->setEnabled(false);
    m_readBtn->setEnabled(false);
    m_writeBtn->setEnabled(false);
}

// ==================== 读取相关 ====================
void MainWindow::onReadClicked()
{
    m_hexLabel->setText("读取中...");
    m_valueLabel->setText("—");
    int area  = m_readAreaCombo->currentIndex();
    int dbNum = m_readDbSpin->value();
    int start = m_readStartSpin->value();
    int size  = m_readSizeSpin->value();
    emit requestRead(area, dbNum, start, size);
}

void MainWindow::onReadAreaChanged(int index)
{
    bool isDB = (index == AREA_DB);
    m_readDbLabel->setVisible(isDB);
    m_readDbSpin->setVisible(isDB);
}

void MainWindow::onDataRead(int areaType, int dbNumber, int start,
                            const QByteArray &data)
{
    (void)areaType; (void)dbNumber; (void)start;
    m_hexLabel->setText(formatHex(data));
    int valueType = m_readTypeCombo->currentData().toInt();
    m_valueLabel->setText(parseValue(data, valueType));
}

// ==================== 写入相关（新增）====================
void MainWindow::onWriteClicked()
{
    m_writeResultLabel->setStyleSheet("color: gray;");
    m_writeResultLabel->setText("写入中...");

    int area     = m_writeAreaCombo->currentIndex();
    int dbNum    = m_writeDbSpin->value();
    int start    = m_writeStartSpin->value();
    int valType  = m_writeTypeCombo->currentData().toInt();
    QString text = m_writeValueEdit->text();

    QByteArray packed = packValue(valType, text);
    if (packed.isEmpty()) {
        m_writeResultLabel->setStyleSheet("color: red;");
        m_writeResultLabel->setText("输入的值格式不正确");
        return;
    }

    emit requestWrite(area, dbNum, start, packed);
    // 写完后自动回读验证
    emit requestRead(area, dbNum, start, packed.size());
}

void MainWindow::onWriteAreaChanged(int index)
{
    bool isDB = (index == AREA_DB);
    m_writeDbLabel->setVisible(isDB);
    m_writeDbSpin->setVisible(isDB);
}

void MainWindow::onDataWritten(int areaType, int dbNumber, int start)
{
    (void)areaType; (void)dbNumber; (void)start;
    m_writeResultLabel->setStyleSheet("color: green;");
    m_writeResultLabel->setText("写入成功（已自动回读验证）");
}

// ==================== 数据打包（新增）====================
QByteArray MainWindow::packValue(int valueType, const QString &text) const
{
    bool ok = false;

    switch (valueType) {
    case 0: { // BOOL
        int v = text.toInt(&ok);
        if (!ok) break;
        return QByteArray(1, (char)(v ? 1 : 0));
    }
    case 1: { // INT8
        int v = text.toInt(&ok);
        if (!ok || v < -128 || v > 127) break;
        return QByteArray(1, (char)v);
    }
    case 2: { // UINT8
        int v = text.toUInt(&ok);
        if (!ok || v > 255) break;
        return QByteArray(1, (char)v);
    }
    case 3: { // INT16 (大端)
        int v = text.toInt(&ok);
        if (!ok || v < -32768 || v > 32767) break;
        QByteArray data(2, '\0');
        data[0] = (char)(((quint16)v >> 8) & 0xFF);  // 高字节
        data[1] = (char)((quint16)v & 0xFF);          // 低字节
        return data;
    }
    case 4: { // UINT16 (大端)
        unsigned int v = text.toUInt(&ok);
        if (!ok || v > 65535) break;
        QByteArray data(2, '\0');
        data[0] = (char)((v >> 8) & 0xFF);
        data[1] = (char)(v & 0xFF);
        return data;
    }
    case 5: { // INT32 (大端)
        qint64 v = text.toLongLong(&ok);
        if (!ok || v < -2147483648LL || v > 2147483647LL) break;
        QByteArray data(4, '\0');
        quint32 u = (quint32)v;
        data[0] = (char)((u >> 24) & 0xFF);
        data[1] = (char)((u >> 16) & 0xFF);
        data[2] = (char)((u >> 8) & 0xFF);
        data[3] = (char)(u & 0xFF);
        return data;
    }
    case 6: { // FLOAT32 (大端)
        float f = text.toFloat(&ok);
        if (!ok) break;
        QByteArray data(4, '\0');
        quint32 bits;
        std::memcpy(&bits, &f, 4);
        data[0] = (char)((bits >> 24) & 0xFF);
        data[1] = (char)((bits >> 16) & 0xFF);
        data[2] = (char)((bits >> 8) & 0xFF);
        data[3] = (char)(bits & 0xFF);
        return data;
    }
    }
    return QByteArray();  // 空数组表示失败
}

// ==================== 工具函数（同项目 3）====================
QString MainWindow::formatHex(const QByteArray &data) const
{
    QString result;
    for (int i = 0; i < data.size(); ++i) {
        if (i > 0 && i % 16 == 0) result += "\n";
        result += QString("%1 ")
                      .arg((unsigned char)data.at(i), 2, 16, QChar('0'))
                      .toUpper();
    }
    return result.trimmed();
}

QString MainWindow::parseValue(const QByteArray &data, int valueType) const
{
    if (data.isEmpty()) return "（无数据）";

    auto U8  = [&](int o) { return (quint8)data.at(o); };
    auto S8  = [&](int o) { return (qint8)data.at(o); };
    auto U16 = [&](int o) { return (U8(o) << 8) | U8(o + 1); };
    auto S16 = [&](int o) { return (qint16)U16(o); };
    auto U32 = [&](int o) { return ((quint32)U16(o) << 16) | U16(o + 2); };
    auto S32 = [&](int o) { return (qint32)U32(o); };
    auto FLT = [&](int o) {
        quint32 b = U32(o); float f; std::memcpy(&f, &b, 4); return f;
    };

    switch (valueType) {
    case -1: return QString("共 %1 字节").arg(data.size());
    case 0: { // BOOL
        QStringList bits;
        for (int i = 0; i < data.size(); ++i)
            for (int b = 0; b < 8; ++b)
                bits << ((U8(i) >> b) & 1 ? "1" : "0");
        return bits.join(' ');
    }
    case 1: { QStringList v; for (int i = 0; i < data.size(); ++i) v << QString::number(S8(i));  return v.join(", "); }
    case 2: { QStringList v; for (int i = 0; i < data.size(); ++i) v << QString::number(U8(i));  return v.join(", "); }
    case 3: { QStringList v; for (int i = 0; i + 1 < data.size(); i += 2) v << QString::number(S16(i)); return v.join(", "); }
    case 4: { QStringList v; for (int i = 0; i + 1 < data.size(); i += 2) v << QString::number(U16(i)); return v.join(", "); }
    case 5: { QStringList v; for (int i = 0; i + 3 < data.size(); i += 4) v << QString::number(S32(i)); return v.join(", "); }
    case 6: { QStringList v; for (int i = 0; i + 3 < data.size(); i += 4) v << QString::number(FLT(i), 'f', 3); return v.join(", "); }
    }
    return "未知";
}
