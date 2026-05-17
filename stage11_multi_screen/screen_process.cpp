#include "screen_process.h"
#include "sim_data_manager.h"
#include "tank_widget.h"
#include "pump_widget.h"
#include "valve_widget.h"
#include "pipe_widget.h"
#include "value_display.h"
#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

ScreenProcess::ScreenProcess(QWidget *parent) : HmiScreen(parent)
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 8, 12, 8);

    // Title
    auto *title = new QLabel("流程画面 / Process Flow", this);
    title->setStyleSheet("color: #aaa; font-size: 11px; font-weight: bold;");
    mainLayout->addWidget(title);

    // Canvas frame
    auto *canvas = new QFrame(this);
    canvas->setStyleSheet("QFrame { background: #202328; "
                          "border: 2px solid #3a3d43; border-radius: 8px; }");
    canvas->setMinimumSize(600, 320);
    mainLayout->addWidget(canvas, 1);

    setupProcessWidgets();
    setupValueDisplays();

    auto *hint = new QLabel("实时流程数据 | Tank → Pump → Valve 流向", this);
    hint->setStyleSheet("color: #666; font-size: 10px;");
    hint->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(hint);
}

void ScreenProcess::setupProcessWidgets()
{
    // canvas is the second child (index 1 in layout)
    auto *canvas = qobject_cast<QFrame *>(
        qobject_cast<QVBoxLayout *>(layout())->itemAt(1)->widget());

    // Tank (left)
    m_tank = new TankWidget(canvas);
    m_tank->setGeometry(30, 30, 160, 240);

    // Pump (center)
    m_pump = new PumpWidget(canvas);
    m_pump->setGeometry(310, 80, 160, 160);

    // Valve (right)
    m_valve = new ValveWidget(canvas);
    m_valve->setGeometry(590, 60, 140, 180);

    // Pipes: Tank→Pump, Pump→Valve
    m_pipeH1 = new PipeWidget(PipeWidget::Horizontal, canvas);
    m_pipeH1->setGeometry(190, 130, 120, 40);

    m_pipeH2 = new PipeWidget(PipeWidget::Horizontal, canvas);
    m_pipeH2->setGeometry(470, 130, 120, 40);
}

void ScreenProcess::setupValueDisplays()
{
    // Bottom value display row — place in main layout below canvas
    auto *mainLayout = qobject_cast<QVBoxLayout *>(layout());
    auto *dispRow = new QHBoxLayout;
    dispRow->setSpacing(10);
    dispRow->addStretch();

    auto makeDisp = [&](const QString &label, const QString &unit) -> ValueDisplay * {
        auto *vd = new ValueDisplay(this);
        vd->setLabel(label);
        vd->setUnit(unit);
        dispRow->addWidget(vd);
        return vd;
    };

    m_dispTemp  = makeDisp("TEMP",  "°C");
    m_dispPress = makeDisp("PRESS", "MPa");
    m_dispTank  = makeDisp("TANK",  "%");
    m_dispFlow  = makeDisp("FLOW",  "m³/h");
    m_dispPump  = makeDisp("PUMP",  "%");
    m_dispValve = makeDisp("VALVE", "%");

    dispRow->addStretch();
    mainLayout->addLayout(dispRow);
}

void ScreenProcess::onEnter()
{
    // initial values
    onTick();
}

void ScreenProcess::onTick()
{
    if (!simData()) return;

    double tank  = simData()->value("TANK");
    double pump  = simData()->value("PUMP");
    double valve = simData()->value("VALVE");

    // Scale values to widget ranges
    m_tank->setLevel(qBound(0.0, (tank / 40.0) * 100, 100.0));
    m_pump->setSpeed(qBound(0.0, (pump / 100.0) * 100, 100.0));
    m_valve->setOpening(qBound(0.0, (valve / 100.0) * 100, 100.0));

    // Pipe flow animation: start when pump > 5% and valve > 5%
    double flowing = (pump > 5 && valve > 5);
    m_pipeH1->setFlowing(flowing);
    m_pipeH2->setFlowing(flowing);

    m_dispTemp->setValue(simData()->value("TEMP"));
    m_dispPress->setValue(simData()->value("PRESS"));
    m_dispTank->setValue(simData()->value("TANK"));
    m_dispFlow->setValue(simData()->value("FLOW"));
    m_dispPump->setValue(simData()->value("PUMP"));
    m_dispValve->setValue(simData()->value("VALVE"));
}
