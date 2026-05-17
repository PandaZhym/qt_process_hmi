#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFrame>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    applyDarkTheme();
    setupUi();
    setupWorker();
    setWindowTitle("Process HMI — Stage 6");
}

MainWindow::~MainWindow()
{
    m_thread->quit();
    m_thread->wait(3000);
}

// ==================== 深色主题 ====================
void MainWindow::applyDarkTheme()
{
    setStyleSheet(R"(
        QMainWindow, QWidget {
            background-color: #1a1d23;
            color: #d0d0d0;
            font-family: "Segoe UI", "Microsoft YaHei";
        }
        QGroupBox {
            border: 1px solid #3a3d43;
            border-radius: 6px;
            margin-top: 14px;
            padding-top: 16px;
            font-weight: bold;
            color: #a0a0a0;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 12px;
        }
        QLineEdit {
            background: #2a2d33;
            border: 1px solid #4a4d53;
            border-radius: 4px;
            padding: 5px 8px;
            color: #e0e0e0;
        }
        QLineEdit:focus { border-color: #4a90d9; }
        QPushButton {
            background: #2d5a8c;
            border: none;
            border-radius: 4px;
            padding: 6px 16px;
            color: white;
            font-weight: bold;
        }
        QPushButton:hover { background: #3a6ea8; }
        QPushButton:pressed { background: #1e4570; }
        QPushButton:disabled { background: #3a3d43; color: #686868; }
        QSpinBox, QComboBox {
            background: #2a2d33;
            border: 1px solid #4a4d53;
            border-radius: 4px;
            padding: 4px 6px;
            color: #e0e0e0;
        }
        QLabel { color: #c0c0c0; }
    )");
}

// ==================== UI 构建 ====================
void MainWindow::setupUi()
{
    QWidget *central = new QWidget;
    setCentralWidget(central);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(12, 8, 12, 8);
    mainLayout->setSpacing(8);

    // ---- 第一行：连接控制 ----
    QHBoxLayout *connRow = new QHBoxLayout;
    connRow->setSpacing(8);

    connRow->addWidget(new QLabel("IP:"));
    m_ipEdit = new QLineEdit("192.168.0.1");
    m_ipEdit->setFixedWidth(140);
    connRow->addWidget(m_ipEdit);

    connRow->addWidget(new QLabel("Rack:"));
    m_rackSpin = new QSpinBox;
    m_rackSpin->setRange(0, 31);
    m_rackSpin->setValue(0);
    m_rackSpin->setFixedWidth(50);
    connRow->addWidget(m_rackSpin);

    connRow->addWidget(new QLabel("Slot:"));
    m_slotSpin = new QSpinBox;
    m_slotSpin->setRange(0, 31);
    m_slotSpin->setValue(1);
    m_slotSpin->setFixedWidth(50);
    connRow->addWidget(m_slotSpin);

    connRow->addWidget(new QLabel("DB:"));
    m_dbSpin = new QSpinBox;
    m_dbSpin->setRange(1, 65535);
    m_dbSpin->setValue(1);
    m_dbSpin->setFixedWidth(60);
    connRow->addWidget(m_dbSpin);

    m_connBtn = new QPushButton("连接");
    connRow->addWidget(m_connBtn);

    m_disconnBtn = new QPushButton("断开");
    m_disconnBtn->setEnabled(false);
    connRow->addWidget(m_disconnBtn);

    m_statusLabel = new QLabel("◉ 未连接");
    m_statusLabel->setStyleSheet("color: #888; font-weight: bold;");
    connRow->addWidget(m_statusLabel);

    m_cpuLabel = new QLabel("");
    m_cpuLabel->setStyleSheet("font-weight: bold;");
    connRow->addWidget(m_cpuLabel);

    connRow->addStretch();
    mainLayout->addLayout(connRow);

    // ---- 第二行：仪表数据栏 ----
    QHBoxLayout *instrRow = new QHBoxLayout;
    instrRow->setSpacing(10);

    m_tempDisp = new ValueDisplay;
    m_tempDisp->setLabel("温度");
    m_tempDisp->setUnit("°C");
    m_tempDisp->setDecimals(1);
    m_tempDisp->setAlarmLimits(0, 80);
    m_tempDisp->setFixedSize(180, 100);
    instrRow->addWidget(m_tempDisp);

    m_pressDisp = new ValueDisplay;
    m_pressDisp->setLabel("压力");
    m_pressDisp->setUnit("MPa");
    m_pressDisp->setDecimals(3);
    m_pressDisp->setAlarmLimits(0, 2.5);
    m_pressDisp->setFixedSize(180, 100);
    instrRow->addWidget(m_pressDisp);

    m_flowDisp = new ValueDisplay;
    m_flowDisp->setLabel("流量");
    m_flowDisp->setUnit("m³/h");
    m_flowDisp->setDecimals(1);
    m_flowDisp->setAlarmLimits(0, 100);
    m_flowDisp->setFixedSize(180, 100);
    instrRow->addWidget(m_flowDisp);

    instrRow->addStretch();
    mainLayout->addLayout(instrRow);

    // ---- 第三行：流程画面 ----
    QFrame *processFrame = new QFrame;
    processFrame->setStyleSheet("QFrame { background: #202328; border: 2px solid #3a3d43; border-radius: 8px; }");
    processFrame->setMinimumHeight(380);
    mainLayout->addWidget(processFrame, 1);

    // 流程画面画布（绝对定位子控件）
    m_tank = new TankWidget(processFrame);
    m_tank->setLabel("原料罐 T-101");
    m_tank->setAlarmLimits(10, 90);
    m_tank->setGeometry(30, 30, 150, 310);

    m_pipe1 = new PipeWidget(PipeWidget::Horizontal, processFrame);
    m_pipe1->setGeometry(180, 175, 100, 24);

    m_pump = new PumpWidget(processFrame);
    m_pump->setLabel("进料泵 P-101");
    m_pump->setGeometry(280, 110, 130, 140);

    m_pipe2 = new PipeWidget(PipeWidget::Horizontal, processFrame);
    m_pipe2->setGeometry(410, 175, 100, 24);

    m_valve = new ValveWidget(processFrame);
    m_valve->setLabel("调节阀 V-101");
    m_valve->setGeometry(510, 120, 110, 110);

    m_pipe3 = new PipeWidget(PipeWidget::Horizontal, processFrame);
    m_pipe3->setGeometry(620, 175, 100, 24);

    // ---- 信号连接 ----
    connect(m_connBtn, &QPushButton::clicked, this, &MainWindow::onConnectClicked);
    connect(m_disconnBtn, &QPushButton::clicked, this, &MainWindow::onDisconnectClicked);

    resize(1050, 680);
}

// ==================== 线程 / Worker ====================
void MainWindow::setupWorker()
{
    m_thread = new QThread(this);
    m_worker = new PlcWorker();
    m_worker->moveToThread(m_thread);

    connect(m_worker, &PlcWorker::connected, this, &MainWindow::onConnected);
    connect(m_worker, &PlcWorker::disconnected, this, &MainWindow::onDisconnected);
    connect(m_worker, &PlcWorker::errorOccurred, this, &MainWindow::onError);
    connect(m_worker, &PlcWorker::processDataUpdated,
            this, &MainWindow::onProcessDataUpdated);
    connect(m_worker, &PlcWorker::cpuStatusChanged,
            this, &MainWindow::onCpuStatusChanged);

    m_thread->start();
}

// ==================== 槽函数 ====================
void MainWindow::onConnectClicked()
{
    QString ip = m_ipEdit->text().trimmed();
    int rack = m_rackSpin->value();
    int slot = m_slotSpin->value();
    int db   = m_dbSpin->value();

    m_statusLabel->setText("连接中...");
    m_statusLabel->setStyleSheet("color: #c90; font-weight: bold;");

    QMetaObject::invokeMethod(m_worker, [this, ip, rack, slot, db]() {
        m_worker->setDataBlock(db);
        m_worker->connectToPlc(ip, rack, slot);
    });
}

void MainWindow::onDisconnectClicked()
{
    QMetaObject::invokeMethod(m_worker, "disconnectFromPlc",
                              Qt::QueuedConnection);
}

void MainWindow::onConnected()
{
    m_statusLabel->setText("◉ 已连接");
    m_statusLabel->setStyleSheet("color: #0c0; font-weight: bold;");
    m_connBtn->setEnabled(false);
    m_disconnBtn->setEnabled(true);
    m_worker->setPollInterval(500);
}

void MainWindow::onDisconnected()
{
    m_statusLabel->setText("◉ 未连接");
    m_statusLabel->setStyleSheet("color: #888; font-weight: bold;");
    m_connBtn->setEnabled(true);
    m_disconnBtn->setEnabled(false);
    m_cpuLabel->setText("");

    m_tank->setLevel(0);
    m_pump->setRunning(false);
    m_valve->setOpening(0);
    m_pipe1->setFlowing(false);
    m_pipe2->setFlowing(false);
    m_pipe3->setFlowing(false);
}

void MainWindow::onError(int code, const QString &msg)
{
    m_statusLabel->setText(QString("错误 %1").arg(code));
    m_statusLabel->setStyleSheet("color: #f00; font-weight: bold;");
}

void MainWindow::onProcessDataUpdated(qreal tankLevel, qreal pumpSpeed, bool pumpRun,
                                      qreal valveOpen, qreal temp,
                                      qreal pressure, qreal flow)
{
    m_tank->setLevel(tankLevel);
    m_pump->setSpeed(pumpSpeed);
    m_pump->setRunning(pumpRun);
    m_valve->setOpening(valveOpen);
    m_pipe1->setFlowing(pumpRun && tankLevel > 0);
    m_pipe2->setFlowing(pumpRun && valveOpen > 5);
    m_pipe3->setFlowing(pumpRun && valveOpen > 5);

    m_tempDisp->setValue(temp);
    m_pressDisp->setValue(pressure);
    m_flowDisp->setValue(flow);
}

void MainWindow::onCpuStatusChanged(int status)
{
    m_cpuLabel->setText(status == 0x08 ? "CPU: RUN"
                      : status == 0x04 ? "CPU: STOP"
                      : QString("CPU: 0x%1").arg(status, 2, 16, QChar('0')));

    QString color = status == 0x08 ? "#0c0"
                  : status == 0x04 ? "#f00" : "#c90";
    m_cpuLabel->setStyleSheet(QString("color: %1; font-weight: bold;").arg(color));
}
