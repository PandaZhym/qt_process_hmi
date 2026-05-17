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

    m_simTimer = new QTimer(this);
    m_simTimer->setInterval(80);
    connect(m_simTimer, &QTimer::timeout, this, &TestWindow::tick);
    m_simTimer->start();

    setWindowTitle("Stage8 — SVG + QPainter Comparison Test");
}

void TestWindow::applyTheme()
{
    setStyleSheet(R"(
        QWidget {
            background-color: #1a1d23;
            color: #c0c0c0;
            font-family: "Segoe UI", "Microsoft YaHei";
        }
        QLabel { color: #c0c0c0; }
    )");
}

void TestWindow::setupUi()
{
    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(14, 10, 14, 10);
    root->setSpacing(6);

    QLabel *title = new QLabel("Process HMI - SVG + QPainter 混合渲染测试");
    title->setStyleSheet("font-size: 16px; font-weight: bold; color: #e0e0e0;");
    root->addWidget(title);

    // 仪表栏
    QHBoxLayout *instrRow = new QHBoxLayout;
    instrRow->setSpacing(12);

    m_tempDisp = new ValueDisplay;
    m_tempDisp->setLabel("温度");  m_tempDisp->setUnit("°C");
    m_tempDisp->setDecimals(1);     m_tempDisp->setAlarmLimits(0, 80);
    m_tempDisp->setFixedSize(200, 110);
    instrRow->addWidget(m_tempDisp);

    m_pressDisp = new ValueDisplay;
    m_pressDisp->setLabel("压力");   m_pressDisp->setUnit("MPa");
    m_pressDisp->setDecimals(3);     m_pressDisp->setAlarmLimits(0, 2.5);
    m_pressDisp->setFixedSize(200, 110);
    instrRow->addWidget(m_pressDisp);

    m_flowDisp = new ValueDisplay;
    m_flowDisp->setLabel("流量");    m_flowDisp->setUnit("m³/h");
    m_flowDisp->setDecimals(1);      m_flowDisp->setAlarmLimits(0, 100);
    m_flowDisp->setFixedSize(200, 110);
    instrRow->addWidget(m_flowDisp);

    instrRow->addStretch();
    root->addLayout(instrRow);

    // 流程画面
    QFrame *canvas = new QFrame;
    canvas->setStyleSheet("QFrame { background: #202328; border: 2px solid #3a3d43; border-radius: 8px; }");
    canvas->setMinimumHeight(420);

    m_tank = new TankWidget(canvas);
    m_tank->setLabel("原料罐 T-101");
    m_tank->setAlarmLimits(10, 90);
    m_tank->setSetpoint(70);
    m_tank->setGeometry(30, 20, 170, 350);

    m_pipe1 = new PipeWidget(PipeWidget::Horizontal, canvas);
    m_pipe1->setGeometry(200, 180, 115, 24);

    m_pump = new PumpWidget(canvas);
    m_pump->setLabel("进料泵 P-101");
    m_pump->setGeometry(315, 120, 130, 150);

    m_pipe2 = new PipeWidget(PipeWidget::Horizontal, canvas);
    m_pipe2->setGeometry(445, 180, 115, 24);

    m_valve = new ValveWidget(canvas);
    m_valve->setLabel("调节阀 V-101");
    m_valve->setGeometry(560, 120, 120, 140);

    m_pipe3 = new PipeWidget(PipeWidget::Horizontal, canvas);
    m_pipe3->setGeometry(680, 180, 115, 24);

    m_tank->setLevel(65);
    m_pump->setSpeed(75);
    m_valve->setOpening(60);
    m_pipe2->setFlowing(true);
    m_pipe3->setFlowing(true);

    root->addWidget(canvas, 1);

    QLabel *hint = new QLabel("SVG 静态装饰 (QSvgRenderer) + QPainter 动态叠加 | 模拟数据驱动");
    hint->setStyleSheet("color: #686868; font-size: 11px;");
    hint->setAlignment(Qt::AlignCenter);
    root->addWidget(hint);

    resize(1050, 700);
}

void TestWindow::tick()
{
    m_simPhase += 0.03;

    qreal tankLevel = 62 + 22 * qSin(m_simPhase * 0.7);
    m_tank->setLevel(tankLevel);

    qreal pumpSpeed = 50 + 45 * (tankLevel / 100.0);
    if (tankLevel < 15) pumpSpeed = 0;
    m_pump->setSpeed(pumpSpeed);
    m_pump->setRunning(pumpSpeed > 0);

    qreal valveOpening = 60 + 28 * qSin(m_simPhase * 0.9 + 0.5);
    m_valve->setOpening(valveOpening);

    bool flow1 = pumpSpeed > 0 && tankLevel > 0;
    bool flow2 = flow1 && valveOpening > 5;
    m_pipe1->setFlowing(flow1);
    m_pipe2->setFlowing(flow2);
    m_pipe3->setFlowing(flow2);

    m_tempDisp->setValue(55 + 30 * qSin(m_simPhase * 0.5));
    m_pressDisp->setValue(1.2 + 1.0 * qSin(m_simPhase * 0.6 + 0.3));
    m_flowDisp->setValue(45 + 35 * qSin(m_simPhase * 0.8 + 0.1));
}
