#include "testwindow.h"
#include "i_protocol.h"
#include "simulated_adapter.h"
#include "tag_manager.h"
#include "value_display.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QDateTime>

TestWindow::TestWindow(QWidget *parent) : QWidget(parent)
{
    setupUi();
    setupProtocol();

    resize(1000, 620);
    setWindowTitle("Stage12 — Protocol Abstraction Interface");
}

TestWindow::~TestWindow()
{
    if (m_protocol) {
        m_protocol->stopPolling();
        m_protocol->disconnect();
    }
}

// ── UI ───────────────────────────────────────────────────────────

void TestWindow::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(8);

    // ── Top bar: protocol selector + connect ─────────────
    auto *topBar = new QHBoxLayout;

    topBar->addWidget(new QLabel("协议:", this));

    m_comboProtocol = new QComboBox(this);
    m_comboProtocol->addItem("Simulated (SIM)");
    m_comboProtocol->setMinimumWidth(150);
    connect(m_comboProtocol, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TestWindow::onProtocolChanged);
    topBar->addWidget(m_comboProtocol);

    topBar->addSpacing(16);

    m_btnConnect = new QPushButton("连接", this);
    m_btnConnect->setStyleSheet(
        "QPushButton { background: #2a5a3a; border: 1px solid #3a7a4a; }"
        "QPushButton:hover { background: #3a7a4a; }");
    connect(m_btnConnect, &QPushButton::clicked, this, &TestWindow::onConnect);
    topBar->addWidget(m_btnConnect);

    m_btnDisconnect = new QPushButton("断开", this);
    m_btnDisconnect->setEnabled(false);
    m_btnDisconnect->setStyleSheet(
        "QPushButton { background: #5a2a2a; border: 1px solid #7a3a3a; }"
        "QPushButton:hover { background: #7a3a3a; }"
        "QPushButton:disabled { background: #333; border-color: #444; color: #777; }");
    connect(m_btnDisconnect, &QPushButton::clicked, this, &TestWindow::onDisconnect);
    topBar->addWidget(m_btnDisconnect);

    topBar->addSpacing(12);

    // Status indicator
    m_statusLed = new QLabel(this);
    m_statusLed->setFixedSize(12, 12);
    m_statusLed->setStyleSheet(
        "background: #555; border-radius: 6px; border: 1px solid #444;");
    topBar->addWidget(m_statusLed);

    m_statusText = new QLabel("未连接", this);
    m_statusText->setStyleSheet("color: #999;");
    topBar->addWidget(m_statusText);

    topBar->addStretch();

    auto *infoLabel = new QLabel("IProtocol 多态演示", this);
    infoLabel->setStyleSheet("color: #666; font-size: 10px;");
    topBar->addWidget(infoLabel);

    mainLayout->addLayout(topBar);

    // ── Value display grid ───────────────────────────────
    auto *dispGroup = new QGroupBox("标签值 / Tag Values", this);
    auto *dispGrid = new QHBoxLayout(dispGroup);
    dispGrid->setSpacing(10);
    dispGrid->addStretch();

    QStringList tags  = {"TEMP", "PRESS", "TANK", "FLOW", "PUMP", "VALVE"};
    QStringList units = {"°C", "MPa", "%", "m³/h", "%", "%"};

    for (int i = 0; i < tags.size(); ++i) {
        auto *vd = new ValueDisplay(this);
        vd->setLabel(tags[i]);
        vd->setUnit(units[i]);
        vd->setMinimumWidth(130);
        m_displays.append(vd);
        dispGrid->addWidget(vd);
    }
    dispGrid->addStretch();
    mainLayout->addWidget(dispGroup);

    // ── Log ──────────────────────────────────────────────
    auto *logGroup = new QGroupBox("事件日志 / Event Log", this);
    auto *logLayout = new QVBoxLayout(logGroup);

    m_logOutput = new QTextEdit(this);
    m_logOutput->setReadOnly(true);
    m_logOutput->setMaximumHeight(180);
    logLayout->addWidget(m_logOutput);

    mainLayout->addWidget(logGroup);

    appendLog("Stage12 启动 — 等待协议连接");
}

// ── Protocol wiring ────────────────────────────────────────────

void TestWindow::setupProtocol()
{
    m_tagManager = new TagManager(this);

    connect(m_tagManager, &TagManager::tagChanged,
            this, &TestWindow::onTagChanged);

    // Default: Simulated
    onProtocolChanged(0);
}

void TestWindow::setupMappings(IProtocol *protocol)
{
    // Map all 6 tags
    QVector<TagMapping> mappings;
    QStringList tags = {"TEMP", "PRESS", "TANK", "FLOW", "PUMP", "VALVE"};
    for (int i = 0; i < tags.size(); ++i) {
        TagMapping m;
        m.tagName = tags[i];
        m.dbNum   = 1;
        m.start   = i * 4;     // 0, 4, 8, ...
        m.size    = 4;
        m.valType = 6;         // FLOAT32
        mappings.append(m);
    }
    protocol->setMappings(mappings);
}

// ── Protocol switching ──────────────────────────────────────────

void TestWindow::onProtocolChanged(int index)
{
    if (m_protocol) {
        m_protocol->stopPolling();
        m_protocol->disconnect();
        m_protocol->deleteLater();
        m_protocol = nullptr;
    }

    // Disconnect old signal connections (auto on delete)

    switch (index) {
    case 0:
    default:
        m_protocol = new SimulatedAdapter(this);
        break;
    }

    // Wire signals
    connect(m_protocol, &IProtocol::connected,        this, &TestWindow::onConnected);
    connect(m_protocol, &IProtocol::disconnected,     this, &TestWindow::onDisconnected);
    connect(m_protocol, &IProtocol::errorOccurred,    this, &TestWindow::onError);
    connect(m_protocol, &IProtocol::tagValuesReady,   this, &TestWindow::onTagValues);

    appendLog(QString("协议切换: %1").arg(m_protocol->protocolName()));

    m_btnConnect->setEnabled(true);
    m_btnDisconnect->setEnabled(false);
    m_statusLed->setStyleSheet(
        "background: #555; border-radius: 6px; border: 1px solid #444;");
    m_statusText->setText("未连接");

    // Reset displays
    for (auto *vd : m_displays)
        vd->setValue(0);
}

// ── Connect / Disconnect ───────────────────────────────────────

void TestWindow::onConnect()
{
    if (!m_protocol) return;

    setupMappings(m_protocol);

    m_btnConnect->setEnabled(false);
    m_statusText->setText("连接中...");
    m_statusLed->setStyleSheet(
        "background: #aa8800; border-radius: 6px; border: 1px solid #774400;");

    appendLog(QString("[%1] 正在连接...").arg(m_protocol->protocolName()));
    m_protocol->connectTo(QVariantMap());  // no params needed for SIM
}

void TestWindow::onDisconnect()
{
    if (!m_protocol) return;
    m_protocol->stopPolling();
    m_protocol->disconnect();
}

// ── Protocol callbacks ──────────────────────────────────────────

void TestWindow::onConnected()
{
    if (!m_protocol) return;

    m_btnDisconnect->setEnabled(true);
    m_statusLed->setStyleSheet(
        "background: #44cc44; border-radius: 6px; border: 1px solid #226622;");

    QString name = m_protocol->protocolName();
    m_statusText->setText(QString("已连接 [%1]").arg(name));
    appendLog(QString("[%1] 连接成功").arg(name));

    // Start polling
    m_protocol->startPolling(200);
    appendLog(QString("[%1] 开始轮询 (200ms)").arg(name));
}

void TestWindow::onDisconnected()
{
    m_btnConnect->setEnabled(true);
    m_btnDisconnect->setEnabled(false);
    m_statusLed->setStyleSheet(
        "background: #555; border-radius: 6px; border: 1px solid #444;");
    m_statusText->setText("未连接");

    QString name = m_protocol ? m_protocol->protocolName() : "?";
    appendLog(QString("[%1] 已断开").arg(name));
}

void TestWindow::onError(const QString &msg)
{
    m_btnConnect->setEnabled(true);
    m_btnDisconnect->setEnabled(false);
    m_statusLed->setStyleSheet(
        "background: #cc4444; border-radius: 6px; border: 1px solid #662222;");
    m_statusText->setText("错误");

    QString name = m_protocol ? m_protocol->protocolName() : "?";
    appendLog(QString("[%1] 错误: %2").arg(name, msg));
}

void TestWindow::onTagValues(const QHash<QString, QVariant> &values)
{
    if (!m_tagManager) return;

    for (auto it = values.begin(); it != values.end(); ++it)
        m_tagManager->updateTag(it.key(), it.value(), true);
}

void TestWindow::onTagChanged(const QString &name, const QVariant &value, bool valid)
{
    QStringList tags = {"TEMP", "PRESS", "TANK", "FLOW", "PUMP", "VALVE"};
    for (int i = 0; i < tags.size() && i < m_displays.size(); ++i) {
        if (name == tags[i] && valid)
            m_displays[i]->setValue(value.toDouble());
    }
}

// ── Log ─────────────────────────────────────────────────────────

void TestWindow::appendLog(const QString &msg)
{
    QString ts = QDateTime::currentDateTime().toString("HH:mm:ss");
    m_logOutput->append(QString("[%1] %2").arg(ts, msg));
}
