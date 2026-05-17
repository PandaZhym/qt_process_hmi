#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFrame>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    applyTheme();
    setupUi();
    setupArchitecture();
    bindWidgets();
    setWindowTitle("Process HMI Framework — MVP (TagManager + Snap7Adapter)");
}

MainWindow::~MainWindow()
{
    m_thread->quit();
    m_thread->wait(3000);
}

// ==================== 主题 ====================
void MainWindow::applyTheme()
{
    setStyleSheet(R"(
        QMainWindow, QWidget {
            background-color: #1a1d23;
            color: #c0c0c0;
            font-family: "Segoe UI", "Microsoft YaHei";
        }
        QLineEdit, QSpinBox {
            background: #2a2d33; border: 1px solid #4a4d53;
            border-radius: 4px; padding: 5px 8px; color: #e0e0e0;
        }
        QPushButton {
            background: #2d5a8c; border: none; border-radius: 4px;
            padding: 6px 16px; color: white; font-weight: bold;
        }
        QPushButton:hover { background: #3a6ea8; }
        QPushButton:disabled { background: #3a3d43; color: #686868; }
        QLabel { color: #c0c0c0; }
    )");
}

// ==================== UI 布局 ====================
void MainWindow::setupUi()
{
    QWidget *central = new QWidget;
    setCentralWidget(central);
    QVBoxLayout *root = new QVBoxLayout(central);
    root->setContentsMargins(12, 8, 12, 8);
    root->setSpacing(8);

    // ---- 第一行：连接控制 + 架构标签 ----
    QHBoxLayout *connRow = new QHBoxLayout;
    connRow->setSpacing(8);

    connRow->addWidget(new QLabel("IP:"));
    m_ipEdit = new QLineEdit("192.168.0.1");
    m_ipEdit->setFixedWidth(140);
    connRow->addWidget(m_ipEdit);

    connRow->addWidget(new QLabel("Rack:"));
    m_rackSpin = new QSpinBox;
    m_rackSpin->setRange(0, 31); m_rackSpin->setValue(0);
    m_rackSpin->setFixedWidth(70);
    connRow->addWidget(m_rackSpin);

    connRow->addWidget(new QLabel("Slot:"));
    m_slotSpin = new QSpinBox;
    m_slotSpin->setRange(0, 31); m_slotSpin->setValue(1);
    m_slotSpin->setFixedWidth(70);
    connRow->addWidget(m_slotSpin);

    m_connBtn = new QPushButton("连接");
    connRow->addWidget(m_connBtn);
    m_disconnBtn = new QPushButton("断开");
    m_disconnBtn->setEnabled(false);
    connRow->addWidget(m_disconnBtn);

    m_statusLbl = new QLabel("◉ 未连接");
    m_statusLbl->setStyleSheet("color: #888; font-weight: bold;");
    connRow->addWidget(m_statusLbl);

    m_cpuLbl = new QLabel("");
    m_cpuLbl->setStyleSheet("font-weight: bold;");
    connRow->addWidget(m_cpuLbl);

    connRow->addStretch();

    // 架构标识
    QLabel *archTag = new QLabel("TagManager → Widgets");
    archTag->setStyleSheet("color: #4a6; font-size: 11px;");
    connRow->addWidget(archTag);

    root->addLayout(connRow);

    // ---- 第二行：仪表栏 ----
    QHBoxLayout *instrRow = new QHBoxLayout;
    instrRow->setSpacing(10);

    m_tempDisp = new ValueDisplay;
    m_tempDisp->setLabel("温度");  m_tempDisp->setUnit("°C");
    m_tempDisp->setDecimals(1);     m_tempDisp->setAlarmLimits(0, 80);
    m_tempDisp->setFixedSize(180, 100);
    instrRow->addWidget(m_tempDisp);

    m_pressDisp = new ValueDisplay;
    m_pressDisp->setLabel("压力");   m_pressDisp->setUnit("MPa");
    m_pressDisp->setDecimals(3);     m_pressDisp->setAlarmLimits(0, 2.5);
    m_pressDisp->setFixedSize(180, 100);
    instrRow->addWidget(m_pressDisp);

    m_flowDisp = new ValueDisplay;
    m_flowDisp->setLabel("流量");    m_flowDisp->setUnit("m³/h");
    m_flowDisp->setDecimals(1);      m_flowDisp->setAlarmLimits(0, 100);
    m_flowDisp->setFixedSize(180, 100);
    instrRow->addWidget(m_flowDisp);

    instrRow->addStretch();
    root->addLayout(instrRow);

    // ---- 第三行：流程画面 ----
    QFrame *canvas = new QFrame;
    canvas->setStyleSheet("QFrame { background: #202328; border: 2px solid #3a3d43; border-radius: 8px; }");
    canvas->setMinimumHeight(380);

    m_tank = new TankWidget(canvas);
    m_tank->setLabel("原料罐 T-101");
    m_tank->setAlarmLimits(10, 90);
    m_tank->setGeometry(30, 30, 150, 310);

    m_pipe1 = new PipeWidget(PipeWidget::Horizontal, canvas);
    m_pipe1->setGeometry(180, 175, 100, 24);

    m_pump = new PumpWidget(canvas);
    m_pump->setLabel("进料泵 P-101");
    m_pump->setGeometry(280, 110, 130, 140);

    m_pipe2 = new PipeWidget(PipeWidget::Horizontal, canvas);
    m_pipe2->setGeometry(410, 175, 100, 24);

    m_valve = new ValveWidget(canvas);
    m_valve->setLabel("调节阀 V-101");
    m_valve->setGeometry(510, 120, 110, 110);

    m_pipe3 = new PipeWidget(PipeWidget::Horizontal, canvas);
    m_pipe3->setGeometry(620, 175, 100, 24);

    root->addWidget(canvas, 1);

    // ---- 信号 ----
    connect(m_connBtn, &QPushButton::clicked, this, &MainWindow::onConnectClicked);
    connect(m_disconnBtn, &QPushButton::clicked, this, &MainWindow::onDisconnectClicked);

    resize(1050, 680);
}

// ==================== 架构组装 ====================
void MainWindow::setupArchitecture()
{
    // Layer 2: TagManager — 主线程
    m_tagMan = new TagManager(this);

    // Layer 1: Snap7Adapter — 移到子线程
    m_thread  = new QThread(this);
    m_adapter = new Snap7Adapter();
    m_adapter->moveToThread(m_thread);

    // 跨线程连接
    connect(m_adapter, &Snap7Adapter::connected,    this, &MainWindow::onConnected);
    connect(m_adapter, &Snap7Adapter::disconnected, this, &MainWindow::onDisconnected);
    connect(m_adapter, &Snap7Adapter::errorOccurred, this, &MainWindow::onError);
    connect(m_adapter, &Snap7Adapter::tagValuesReady,
            this, &MainWindow::onTagValuesReady);      // 跨线程自动 QueuedConnection

    m_thread->start();
}

// ==================== 控件 ↔ 标签绑定 ====================
void MainWindow::bindWidgets()
{
    // 画面控件订阅 TagManager 的标签变化
    connect(m_tagMan, &TagManager::tagChanged, this, &MainWindow::onTagChanged);
}

void MainWindow::onTagChanged(const QString &name, const QVariant &value, bool valid)
{
    if (!valid) return;

    if (name == "TANK_LEVEL")     m_tank->setLevel(value.toFloat());
    if (name == "PUMP_SPEED")    { m_pump->setSpeed(value.toFloat()); m_pump->setRunning(value.toFloat() > 0); }
    if (name == "VALVE_OPENING") m_valve->setOpening(value.toFloat());
    if (name == "TEMP_VALUE")    m_tempDisp->setValue(value.toFloat());
    if (name == "PRESS_VALUE")   m_pressDisp->setValue(value.toFloat());
    if (name == "FLOW_VALUE")    m_flowDisp->setValue(value.toFloat());

    // 管道状态：泵运转 + 液位 > 0
    bool hasFlow = m_tagMan->tagValue("PUMP_SPEED").toFloat() > 0
                && m_tagMan->tagValue("TANK_LEVEL").toFloat() > 0;
    bool valveFlow = hasFlow && m_valve->opening() > 5;
    m_pipe1->setFlowing(hasFlow);
    m_pipe2->setFlowing(valveFlow);
    m_pipe3->setFlowing(valveFlow);
}

// ==================== 槽：连接 / 断开 ====================
void MainWindow::onConnectClicked()
{
    m_statusLbl->setText("连接中...");
    m_statusLbl->setStyleSheet("color: #c90; font-weight: bold;");

    QString ip  = m_ipEdit->text().trimmed();
    int rack    = m_rackSpin->value();
    int slot    = m_slotSpin->value();

    // 配置标签映射（告诉适配器 PLC 地址 → 标签名）
    QVector<TagMapping> mappings;
    mappings.append({"TANK_LEVEL",    S7AreaDB, 6, 0,  4, 6});  // DB6.DBD0  FLOAT
    mappings.append({"PUMP_SPEED",    S7AreaDB, 6, 4,  4, 6});  // DB6.DBD4  FLOAT
    mappings.append({"VALVE_OPENING", S7AreaDB, 6, 8,  4, 6});  // DB6.DBD8  FLOAT
    mappings.append({"TEMP_VALUE",    S7AreaDB, 6, 12, 4, 6});  // DB6.DBD12 FLOAT
    mappings.append({"PRESS_VALUE",   S7AreaDB, 6, 16, 4, 6});  // DB6.DBD16 FLOAT
    mappings.append({"FLOW_VALUE",    S7AreaDB, 6, 20, 4, 6});  // DB6.DBD20 FLOAT

    QMetaObject::invokeMethod(m_adapter, [this, ip, rack, slot, mappings]() {
        m_adapter->setMappings(mappings);
        m_adapter->connectToPlc(ip, rack, slot);
    });
}

void MainWindow::onDisconnectClicked()
{
    QMetaObject::invokeMethod(m_adapter, "disconnectFromPlc",
                              Qt::QueuedConnection);
}

void MainWindow::onConnected()
{
    m_statusLbl->setText("◉ 已连接");
    m_statusLbl->setStyleSheet("color: #0c0; font-weight: bold;");
    m_connBtn->setEnabled(false);
    m_disconnBtn->setEnabled(true);
    QMetaObject::invokeMethod(m_adapter, "startPolling",
                              Q_ARG(int, 500));
}

void MainWindow::onDisconnected()
{
    m_statusLbl->setText("◉ 未连接");
    m_statusLbl->setStyleSheet("color: #888; font-weight: bold;");
    m_connBtn->setEnabled(true);
    m_disconnBtn->setEnabled(false);
    m_cpuLbl->setText("");
}

void MainWindow::onError(int code, const QString &msg)
{
    m_statusLbl->setText(QString("✗ 错误 %1").arg(code));
    m_statusLbl->setStyleSheet("color: #f00; font-weight: bold;");
}

// ==================== 数据流核心 ====================
void MainWindow::onTagValuesReady(const QHash<QString, QVariant> &values)
{
    // Snap7Adapter(子线程) → TagManager(主线程)
    for (auto it = values.begin(); it != values.end(); ++it)
        m_tagMan->updateTag(it.key(), it.value());

    // CPU 状态
    m_cpuLbl->setText(QString("CPU: RUN"));
    m_cpuLbl->setStyleSheet("color: #0c0; font-weight: bold;");
}
