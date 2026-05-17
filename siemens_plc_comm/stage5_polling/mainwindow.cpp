#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QFont>
#include <cstring>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();

    m_thread = new QThread(this);
    m_worker = new PlcWorker();
    m_worker->moveToThread(m_thread);

    setupConnections();

    m_thread->start();

    // 默认添加 3 行监控项
    onAddMonitorItem();
    onAddMonitorItem();
    onAddMonitorItem();
}

MainWindow::~MainWindow()
{
    m_thread->quit();
    m_thread->wait();
}

// ==================== UI ====================
void MainWindow::setupUi()
{
    setWindowTitle("S7-1200 实时监控");
    resize(720, 620);

    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    // ---------- 连接 ----------
    QGroupBox *connGroup = new QGroupBox("连接");
    QVBoxLayout *connLayout = new QVBoxLayout(connGroup);

    QHBoxLayout *ipRow = new QHBoxLayout();
    m_ipEdit = new QLineEdit("192.168.0.1");
    m_connectBtn = new QPushButton("连接");
    m_disconnectBtn = new QPushButton("断开");
    m_disconnectBtn->setEnabled(false);
    m_cpuLabel = new QLabel("CPU: —");
    m_cpuLabel->setStyleSheet("font-weight: bold;");
    ipRow->addWidget(new QLabel("PLC IP:"));
    ipRow->addWidget(m_ipEdit);
    ipRow->addWidget(m_connectBtn);
    ipRow->addWidget(m_disconnectBtn);
    ipRow->addStretch();
    ipRow->addWidget(m_cpuLabel);
    connLayout->addLayout(ipRow);

    m_statusLabel = new QLabel("未连接");
    QFont sf = m_statusLabel->font();
    sf.setPointSize(12);
    m_statusLabel->setFont(sf);
    connLayout->addWidget(m_statusLabel);
    mainLayout->addWidget(connGroup);

    // ---------- 监控表 ----------
    QGroupBox *monGroup = new QGroupBox("监控地址表（轮询）");
    QVBoxLayout *monLayout = new QVBoxLayout(monGroup);

    m_table = new QTableWidget(0, 6);
    m_table->setHorizontalHeaderLabels(
        {"区域", "DB号", "起始", "类型", "当前值", "状态"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setColumnWidth(0, 70);
    m_table->setColumnWidth(1, 70);
    m_table->setColumnWidth(2, 70);
    m_table->setColumnWidth(3, 80);
    m_table->setColumnWidth(4, 180);
    monLayout->addWidget(m_table);

    QHBoxLayout *tblBtnRow = new QHBoxLayout();
    m_addBtn = new QPushButton("+ 添加");
    m_removeBtn = new QPushButton("- 删除");
    tblBtnRow->addWidget(m_addBtn);
    tblBtnRow->addWidget(m_removeBtn);
    tblBtnRow->addStretch();
    monLayout->addLayout(tblBtnRow);

    // 轮询控制
    QHBoxLayout *pollRow = new QHBoxLayout();
    pollRow->addWidget(new QLabel("轮询间隔(ms):"));
    m_intervalSpin = new QSpinBox();
    m_intervalSpin->setRange(100, 10000);
    m_intervalSpin->setValue(500);
    m_intervalSpin->setSingleStep(100);
    pollRow->addWidget(m_intervalSpin);
    m_startPollBtn = new QPushButton("开始轮询");
    m_startPollBtn->setEnabled(false);
    m_stopPollBtn = new QPushButton("停止");
    m_stopPollBtn->setEnabled(false);
    pollRow->addWidget(m_startPollBtn);
    pollRow->addWidget(m_stopPollBtn);
    pollRow->addStretch();
    monLayout->addLayout(pollRow);
    mainLayout->addWidget(monGroup);

    // ---------- 手动读写 ----------
    QGroupBox *rwGroup = new QGroupBox("手动读写");
    QVBoxLayout *rwLayout = new QVBoxLayout(rwGroup);

    QHBoxLayout *rwParamRow = new QHBoxLayout();
    m_rwAreaCombo = new QComboBox();
    m_rwAreaCombo->addItems({"M 区", "I 区", "Q 区", "DB 块"});
    rwParamRow->addWidget(new QLabel("区域:"));
    rwParamRow->addWidget(m_rwAreaCombo);
    m_rwDbLabel = new QLabel("DB号:");
    m_rwDbSpin = new QSpinBox();
    m_rwDbSpin->setRange(1, 65535);
    m_rwDbSpin->setValue(1);
    m_rwDbLabel->setVisible(false);
    m_rwDbSpin->setVisible(false);
    rwParamRow->addWidget(m_rwDbLabel);
    rwParamRow->addWidget(m_rwDbSpin);
    rwParamRow->addWidget(new QLabel("起始:"));
    m_rwStartSpin = new QSpinBox();
    m_rwStartSpin->setRange(0, 65535);
    rwParamRow->addWidget(m_rwStartSpin);
    rwLayout->addLayout(rwParamRow);

    QHBoxLayout *rwBtnRow = new QHBoxLayout();
    rwBtnRow->addWidget(new QLabel("类型:"));
    m_rwTypeCombo = new QComboBox();
    m_rwTypeCombo->addItem("BOOL",      0);
    m_rwTypeCombo->addItem("INT8",      1);
    m_rwTypeCombo->addItem("UINT8",     2);
    m_rwTypeCombo->addItem("INT16",     3);
    m_rwTypeCombo->addItem("UINT16",    4);
    m_rwTypeCombo->addItem("INT32",     5);
    m_rwTypeCombo->addItem("FLOAT32",   6);
    m_rwTypeCombo->setCurrentIndex(4);  // 默认 INT16（索引变了）
    rwBtnRow->addWidget(m_rwTypeCombo);
    rwBtnRow->addWidget(new QLabel("值:"));
    m_rwValueEdit = new QLineEdit("0");
    rwBtnRow->addWidget(m_rwValueEdit);
    m_readBtn = new QPushButton("读取");
    m_readBtn->setEnabled(false);
    m_writeBtn = new QPushButton("写入");
    m_writeBtn->setEnabled(false);
    rwBtnRow->addWidget(m_readBtn);
    rwBtnRow->addWidget(m_writeBtn);
    rwLayout->addLayout(rwBtnRow);

    QFont monoFont("Consolas", 10);
    m_rwHexLabel = new QLabel("—");
    m_rwHexLabel->setFont(monoFont);
    m_rwHexLabel->setWordWrap(true);
    rwLayout->addWidget(m_rwHexLabel);
    m_rwValueLabel = new QLabel("—");
    m_rwValueLabel->setFont(monoFont);
    rwLayout->addWidget(m_rwValueLabel);
    m_rwResultLabel = new QLabel();
    rwLayout->addWidget(m_rwResultLabel);
    mainLayout->addWidget(rwGroup);
}

// ==================== 信号/槽 ====================
void MainWindow::setupConnections()
{
    // 按钮
    connect(m_connectBtn, &QPushButton::clicked,
            this, &MainWindow::onConnectClicked);
    connect(m_disconnectBtn, &QPushButton::clicked,
            this, &MainWindow::onDisconnectClicked);
    connect(m_readBtn, &QPushButton::clicked,
            this, &MainWindow::onReadClicked);
    connect(m_writeBtn, &QPushButton::clicked,
            this, &MainWindow::onWriteClicked);
    connect(m_addBtn, &QPushButton::clicked,
            this, &MainWindow::onAddMonitorItem);
    connect(m_removeBtn, &QPushButton::clicked,
            this, &MainWindow::onRemoveMonitorItem);
    connect(m_startPollBtn, &QPushButton::clicked,
            this, &MainWindow::onStartPolling);
    connect(m_stopPollBtn, &QPushButton::clicked,
            this, &MainWindow::onStopPolling);

    // 手动读写区域切换
    connect(m_rwAreaCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int idx) {
        bool db = (idx == AREA_DB);
        m_rwDbLabel->setVisible(db);
        m_rwDbSpin->setVisible(db);
    });

    // UI → Worker
    connect(this, &MainWindow::requestConnect,
            m_worker, &PlcWorker::connectToPlc);
    connect(this, &MainWindow::requestDisconnect,
            m_worker, &PlcWorker::disconnectFromPlc);
    connect(this, &MainWindow::requestRead,
            m_worker, &PlcWorker::readArea);
    connect(this, &MainWindow::requestWrite,
            m_worker, &PlcWorker::writeArea);
    connect(this, &MainWindow::requestStartPolling,
            m_worker, &PlcWorker::startPolling);
    connect(this, &MainWindow::requestStopPolling,
            m_worker, &PlcWorker::stopPolling);
    connect(this, &MainWindow::requestSetMonitorItems,
            m_worker, &PlcWorker::setMonitorItems);

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
    connect(m_worker, &PlcWorker::pollingData,
            this, &MainWindow::onPollingData);
    connect(m_worker, &PlcWorker::cpuStatusChanged,
            this, &MainWindow::onCpuStatusChanged);

    connect(m_thread, &QThread::finished,
            m_worker, &QObject::deleteLater);
}

// ==================== 连接 ====================
void MainWindow::onConnectClicked()
{
    m_statusLabel->setText("正在连接...");
    m_connectBtn->setEnabled(false);
    emit requestConnect(m_ipEdit->text(), 0, 1);
}

void MainWindow::onDisconnectClicked()
{
    emit requestStopPolling();
    emit requestDisconnect();
}

void MainWindow::onConnected()
{
    m_statusLabel->setStyleSheet("color: green;");
    m_statusLabel->setText("已连接");
    m_connectBtn->setEnabled(false);
    m_disconnectBtn->setEnabled(true);
    m_startPollBtn->setEnabled(true);
    m_readBtn->setEnabled(true);
    m_writeBtn->setEnabled(true);
}

void MainWindow::onDisconnected()
{
    m_statusLabel->setStyleSheet("color: gray;");
    m_statusLabel->setText("未连接");
    m_cpuLabel->setText("CPU: —");
    m_connectBtn->setEnabled(true);
    m_disconnectBtn->setEnabled(false);
    m_startPollBtn->setEnabled(false);
    m_stopPollBtn->setEnabled(false);
    m_readBtn->setEnabled(false);
    m_writeBtn->setEnabled(false);
    // 恢复轮询表区域，否则重连后无法操作
    m_intervalSpin->setEnabled(true);
    m_addBtn->setEnabled(true);
    m_removeBtn->setEnabled(true);
    m_table->setEnabled(true);
}

void MainWindow::onError(int code, const QString &message)
{
    m_statusLabel->setStyleSheet("color: red;");
    m_statusLabel->setText(QString("错误 %1: %2").arg(code).arg(message));
    m_connectBtn->setEnabled(true);
    m_disconnectBtn->setEnabled(false);
    m_startPollBtn->setEnabled(false);
    m_stopPollBtn->setEnabled(false);
    m_readBtn->setEnabled(false);
    m_writeBtn->setEnabled(false);
}

// ==================== 手动读写 ====================
void MainWindow::onReadClicked()
{
    m_rwHexLabel->setText("...");
    int area  = m_rwAreaCombo->currentIndex();
    int dbNum = m_rwDbSpin->value();
    int start = m_rwStartSpin->value();
    int type  = m_rwTypeCombo->currentData().toInt();
    int size  = (type <= 2) ? 1 : (type <= 4) ? 2 : 4;
    emit requestRead(area, dbNum, start, size);
}

void MainWindow::onDataRead(int areaType, int dbNumber, int start,
                            const QByteArray &data)
{
    (void)areaType; (void)dbNumber; (void)start;
    m_rwHexLabel->setText(formatHex(data));
    int type = m_rwTypeCombo->currentData().toInt();
    m_rwValueLabel->setText(parseValue(data, type));
}

void MainWindow::onWriteClicked()
{
    int area  = m_rwAreaCombo->currentIndex();
    int dbNum = m_rwDbSpin->value();
    int start = m_rwStartSpin->value();
    int type  = m_rwTypeCombo->currentData().toInt();
    QByteArray packed = packValue(type, m_rwValueEdit->text());
    if (packed.isEmpty()) {
        m_rwResultLabel->setStyleSheet("color: red;");
        m_rwResultLabel->setText("值格式错误");
        return;
    }
    emit requestWrite(area, dbNum, start, packed);
    // 回读验证
    emit requestRead(area, dbNum, start, packed.size());
}

void MainWindow::onDataWritten(int areaType, int dbNumber, int start)
{
    (void)areaType; (void)dbNumber; (void)start;
    m_rwResultLabel->setStyleSheet("color: green;");
    m_rwResultLabel->setText("写入成功（已回读验证）");
}

// ==================== 监控表操作 ====================
void MainWindow::onAddMonitorItem()
{
    int row = m_table->rowCount();
    m_table->insertRow(row);

    // 区域
    QComboBox *areaCb = new QComboBox();
    areaCb->addItems({"M", "I", "Q", "DB"});
    m_table->setCellWidget(row, 0, areaCb);

    // DB 号
    QSpinBox *dbSpin = new QSpinBox();
    dbSpin->setRange(1, 65535);
    dbSpin->setValue(1);
    dbSpin->setEnabled(false);
    m_table->setCellWidget(row, 1, dbSpin);

    // 起始地址
    QSpinBox *startSpin = new QSpinBox();
    startSpin->setRange(0, 65535);
    m_table->setCellWidget(row, 2, startSpin);

    // 类型
    QComboBox *typeCb = new QComboBox();
    typeCb->addItem("BOOL",     0);
    typeCb->addItem("INT8",     1);
    typeCb->addItem("UINT8",    2);
    typeCb->addItem("INT16",    3);
    typeCb->addItem("UINT16",   4);
    typeCb->addItem("INT32",    5);
    typeCb->addItem("FLOAT32",  6);
    typeCb->setCurrentIndex(4);
    m_table->setCellWidget(row, 3, typeCb);

    // 当前值（文本标签）
    QLabel *valLabel = new QLabel("—");
    m_table->setCellWidget(row, 4, valLabel);

    // 状态
    QLabel *statLabel = new QLabel("—");
    m_table->setCellWidget(row, 5, statLabel);

    // DB 区域切换
    connect(areaCb, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [dbSpin](int idx) { dbSpin->setEnabled(idx == AREA_DB); });
}

void MainWindow::onRemoveMonitorItem()
{
    int row = m_table->currentRow();
    if (row >= 0)
        m_table->removeRow(row);
}

void MainWindow::syncItemsToWorker()
{
    QVector<MonitorItem> items;
    for (int i = 0; i < m_table->rowCount(); ++i) {
        auto *areaCb  = qobject_cast<QComboBox*>(m_table->cellWidget(i, 0));
        auto *dbSpin  = qobject_cast<QSpinBox*>(m_table->cellWidget(i, 1));
        auto *startSpin = qobject_cast<QSpinBox*>(m_table->cellWidget(i, 2));
        auto *typeCb  = qobject_cast<QComboBox*>(m_table->cellWidget(i, 3));
        if (!areaCb || !startSpin || !typeCb) continue;

        MonitorItem item;
        item.area    = areaCb->currentIndex();
        item.dbNum   = dbSpin ? dbSpin->value() : 0;
        item.start   = startSpin->value();
        item.valType = typeCb->currentData().toInt();
        items.append(item);
    }
    emit requestSetMonitorItems(items);
}

// ==================== 轮询 ====================
void MainWindow::onStartPolling()
{
    syncItemsToWorker();
    emit requestStartPolling(m_intervalSpin->value());

    m_startPollBtn->setEnabled(false);
    m_stopPollBtn->setEnabled(true);
    m_intervalSpin->setEnabled(false);
    m_addBtn->setEnabled(false);
    m_removeBtn->setEnabled(false);
    m_table->setEnabled(false);
}

void MainWindow::onStopPolling()
{
    emit requestStopPolling();

    m_startPollBtn->setEnabled(true);
    m_stopPollBtn->setEnabled(false);
    m_intervalSpin->setEnabled(true);
    m_addBtn->setEnabled(true);
    m_removeBtn->setEnabled(true);
    m_table->setEnabled(true);
}

void MainWindow::onPollingData(int idx, int area, int dbNum, int start,
                               const QByteArray &data)
{
    (void)area; (void)dbNum; (void)start;
    if (idx < 0 || idx >= m_table->rowCount()) return;

    auto *valLabel  = qobject_cast<QLabel*>(m_table->cellWidget(idx, 4));
    auto *statLabel = qobject_cast<QLabel*>(m_table->cellWidget(idx, 5));
    auto *typeCb    = qobject_cast<QComboBox*>(m_table->cellWidget(idx, 3));
    if (!valLabel || !statLabel || !typeCb) return;

    if (data.isEmpty()) {
        statLabel->setText("失败");
        statLabel->setStyleSheet("color: red;");
        valLabel->setText("—");
    } else {
        statLabel->setText("OK");
        statLabel->setStyleSheet("color: green;");
        int valType = typeCb->currentData().toInt();
        valLabel->setText(parseValue(data, valType));
    }
}

void MainWindow::onCpuStatusChanged(int status)
{
    if (status == 0x08) {
        m_cpuLabel->setText("CPU: RUN");
        m_cpuLabel->setStyleSheet("color: green; font-weight: bold;");
    } else if (status == 0x04) {
        m_cpuLabel->setText("CPU: STOP");
        m_cpuLabel->setStyleSheet("color: red; font-weight: bold;");
    } else {
        m_cpuLabel->setText(QString("CPU: 0x%1").arg(status, 2, 16));
        m_cpuLabel->setStyleSheet("color: orange; font-weight: bold;");
    }
}

// ==================== 工具函数 ====================
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
    if (data.isEmpty()) return "—";

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
    case 0: { QStringList v; for (int i = 0; i < data.size(); ++i) for (int b = 0; b < 8; ++b) v << ((U8(i)>>b)&1?"1":"0"); return v.join(' '); }
    case 1: { QStringList v; for (int i = 0; i < data.size(); ++i) v << QString::number(S8(i));  return v.join(", "); }
    case 2: { QStringList v; for (int i = 0; i < data.size(); ++i) v << QString::number(U8(i));  return v.join(", "); }
    case 3: { QStringList v; for (int i = 0; i + 1 < data.size(); i += 2) v << QString::number(S16(i)); return v.join(", "); }
    case 4: { QStringList v; for (int i = 0; i + 1 < data.size(); i += 2) v << QString::number(U16(i)); return v.join(", "); }
    case 5: { QStringList v; for (int i = 0; i + 3 < data.size(); i += 4) v << QString::number(S32(i)); return v.join(", "); }
    case 6: { QStringList v; for (int i = 0; i + 3 < data.size(); i += 4) v << QString::number(FLT(i), 'f', 3); return v.join(", "); }
    }
    return QString("共%1字节").arg(data.size());
}

QByteArray MainWindow::packValue(int valueType, const QString &text) const
{
    bool ok = false;
    switch (valueType) {
    case 0:  { int v = text.toInt(&ok); return ok ? QByteArray(1, (char)(v?1:0)) : QByteArray(); }
    case 1:  { int v = text.toInt(&ok); if (!ok || v < -128 || v > 127) break; return QByteArray(1, (char)v); }
    case 2:  { unsigned int v = text.toUInt(&ok); if (!ok || v > 255) break; return QByteArray(1, (char)v); }
    case 3:  { int v = text.toInt(&ok); if (!ok || v < -32768 || v > 32767) break; QByteArray d(2,'\0'); d[0]=(v>>8)&0xFF; d[1]=v&0xFF; return d; }
    case 4:  { unsigned int v = text.toUInt(&ok); if (!ok || v > 65535) break; QByteArray d(2,'\0'); d[0]=(v>>8)&0xFF; d[1]=v&0xFF; return d; }
    case 5:  { qint64 v = text.toLongLong(&ok); if (!ok||v<-2147483648LL||v>2147483647LL) break; quint32 u=(quint32)v; QByteArray d(4,'\0'); d[0]=(u>>24)&0xFF;d[1]=(u>>16)&0xFF;d[2]=(u>>8)&0xFF;d[3]=u&0xFF; return d; }
    case 6:  { float f = text.toFloat(&ok); if (!ok) break; QByteArray d(4,'\0'); quint32 b; memcpy(&b,&f,4); d[0]=(b>>24)&0xFF;d[1]=(b>>16)&0xFF;d[2]=(b>>8)&0xFF;d[3]=b&0xFF; return d; }
    }
    return QByteArray();
}
