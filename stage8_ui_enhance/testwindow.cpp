#include "testwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QtMath>

TestWindow::TestWindow(QWidget *parent) : QWidget(parent)
{
    applyTheme();
    setupUi();

    // 模拟数据定时器
    m_simTimer = new QTimer(this);
    m_simTimer->setInterval(80);
    connect(m_simTimer, &QTimer::timeout, this, &TestWindow::tick);
    m_simTimer->start();

    setWindowTitle("Stage8 — UI Enhance Test (Pure QPainter)");
}

void TestWindow::applyTheme()
{
    setStyleSheet(R"(
        QWidget {
            background-color: #1a1d23;
            color: #c0c0c0;
            font-family: "Segoe UI", "Microsoft YaHei";
        }
        QLabel {
            color: #c0c0c0;
        }
    )");
}

void TestWindow::setupUi()
{
    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(14, 10, 14, 10);
    root->setSpacing(6);

    // ---- 标题 ----
    QLabel *title = new QLabel("Process HMI — 控件美化测试");
    title->setStyleSheet("font-size: 16px; font-weight: bold; color: #e0e0e0;");
    root->addWidget(title);

    // ---- 仪表栏 ----
    QHBoxLayout *instrRow = new QHBoxLayout;
    instrRow->setSpacing(12);

    m_tempDisp = new ValueDisplay;
    m_tempDisp->setLabel("温度");  m_tempDisp->setUnit("°C");
    m_tempDisp->setDecimals(1);     m_tempDisp->setAlarmLimits(0, 80);
    m_tempDisp->setFixedSize(190, 105);
    instrRow->addWidget(m_tempDisp);

    m_pressDisp = new ValueDisplay;
    m_pressDisp->setLabel("压力");   m_pressDisp->setUnit("MPa");
    m_pressDisp->setDecimals(3);     m_pressDisp->setAlarmLimits(0, 2.5);
    m_pressDisp->setFixedSize(190, 105);
    instrRow->addWidget(m_pressDisp);

    m_flowDisp = new ValueDisplay;
    m_flowDisp->setLabel("流量");    m_flowDisp->setUnit("m³/h");
    m_flowDisp->setDecimals(1);      m_flowDisp->setAlarmLimits(0, 100);
    m_flowDisp->setFixedSize(190, 105);
    instrRow->addWidget(m_flowDisp);

    instrRow->addStretch();
    root->addLayout(instrRow);

    // ---- 流程画面 ----
    QFrame *canvas = new QFrame;
    canvas->setStyleSheet("QFrame { background: #202328; border: 2px solid #3a3d43; border-radius: 8px; }");
    canvas->setMinimumHeight(400);

    m_tank = new TankWidget(canvas);
    m_tank->setLabel("原料罐 T-101");
    m_tank->setAlarmLimits(10, 90);
    m_tank->setSetpoint(70);
    m_tank->setGeometry(30, 30, 160, 330);

    m_pipe1 = new PipeWidget(PipeWidget::Horizontal, canvas);
    m_pipe1->setGeometry(190, 185, 110, 24);

    m_pump = new PumpWidget(canvas);
    m_pump->setLabel("进料泵 P-101");
    m_pump->setGeometry(300, 120, 130, 150);

    m_pipe2 = new PipeWidget(PipeWidget::Horizontal, canvas);
    m_pipe2->setGeometry(430, 185, 110, 24);

    m_valve = new ValveWidget(canvas);
    m_valve->setLabel("调节阀 V-101");
    m_valve->setGeometry(540, 130, 110, 120);

    m_pipe3 = new PipeWidget(PipeWidget::Horizontal, canvas);
    m_pipe3->setGeometry(650, 185, 110, 24);

    // 初始值
    m_tank->setLevel(65);
    m_pump->setSpeed(75);
    m_valve->setOpening(60);
    m_pipe2->setFlowing(true);
    m_pipe3->setFlowing(true);

    root->addWidget(canvas, 1);

    // ---- 说明 ----
    QLabel *hint = new QLabel("模拟数据驱动 | Tank / Pump 用 QPropertyAnimation 动画 | Pipe 用粒子块流动画 | Value 闪烁");
    hint->setStyleSheet("color: #686868; font-size: 11px;");
    hint->setAlignment(Qt::AlignCenter);
    root->addWidget(hint);

    resize(1050, 680);
}

void TestWindow::tick()
{
    m_simPhase += 0.03;

    // 模拟液位波动：在 40~85 之间摆动
    qreal tankLevel = 62 + 22 * qSin(m_simPhase * 0.7);
    m_tank->setLevel(tankLevel);

    // 泵转速跟随液位（液位高则转速高）
    qreal pumpSpeed = 50 + 45 * (tankLevel / 100.0);
    if (tankLevel < 15) pumpSpeed = 0;
    m_pump->setSpeed(pumpSpeed);
    m_pump->setRunning(pumpSpeed > 0);

    // 阀门开度在 30~90 之间摆动
    qreal valveOpening = 60 + 28 * qSin(m_simPhase * 0.9 + 0.5);
    m_valve->setOpening(valveOpening);

    // 管道流动状态
    bool flow1 = pumpSpeed > 0 && tankLevel > 0;
    bool flow2 = flow1 && valveOpening > 5;
    m_pipe1->setFlowing(flow1);
    m_pipe2->setFlowing(flow2);
    m_pipe3->setFlowing(flow2);

    // 仪表盘模拟
    qreal temp = 55 + 30 * qSin(m_simPhase * 0.5);
    qreal press = 1.2 + 1.0 * qSin(m_simPhase * 0.6 + 0.3);
    qreal flow = 45 + 35 * qSin(m_simPhase * 0.8 + 0.1);
    m_tempDisp->setValue(temp);
    m_pressDisp->setValue(press);
    m_flowDisp->setValue(flow);
}
