#include "widget_factory.h"
#include "tank_widget.h"
#include "pump_widget.h"
#include "valve_widget.h"
#include "pipe_widget.h"
#include "value_display.h"
#include <QDebug>

QHash<QString, WidgetFactory::CreatorFunc> &WidgetFactory::creators()
{
    static QHash<QString, CreatorFunc> s_creators;
    return s_creators;
}

void WidgetFactory::ensureInit()
{
    auto &c = creators();
    if (!c.isEmpty()) return;

    c["TankWidget"]   = [](QWidget *p) { return new TankWidget(p); };
    c["PumpWidget"]   = [](QWidget *p) { return new PumpWidget(p); };
    c["ValveWidget"]  = [](QWidget *p) { return new ValveWidget(p); };
    c["PipeWidget_H"] = [](QWidget *p) { return new PipeWidget(PipeWidget::Horizontal, p); };
    c["PipeWidget_V"] = [](QWidget *p) { return new PipeWidget(PipeWidget::Vertical, p); };
    c["ValueDisplay"] = [](QWidget *p) { return new ValueDisplay(p); };
}

QWidget *WidgetFactory::create(const QString &typeName, QWidget *parent)
{
    ensureInit();
    auto &c = creators();
    if (c.contains(typeName))
        return c[typeName](parent);

    qWarning() << "WidgetFactory: unknown type" << typeName;
    return nullptr;
}

void WidgetFactory::applyProperties(const QString &typeName, QWidget *widget,
                                    const QJsonObject &props)
{
    if (!widget) return;

    if (typeName == "ValueDisplay") {
        auto *vd = static_cast<ValueDisplay *>(widget);
        if (props.contains("label"))    vd->setLabel(props["label"].toString());
        if (props.contains("unit"))     vd->setUnit(props["unit"].toString());
        if (props.contains("decimals")) vd->setDecimals(props["decimals"].toInt(1));
        if (props.contains("alarmLow")) vd->setAlarmLimits(props["alarmLow"].toDouble(0),
                                                           props["alarmHigh"].toDouble(100));
        return;
    }

    if (typeName == "TankWidget") {
        auto *tw = static_cast<TankWidget *>(widget);
        if (props.contains("label"))    tw->setLabel(props["label"].toString());
        if (props.contains("alarmLow")) tw->setAlarmLimits(props["alarmLow"].toDouble(10),
                                                           props["alarmHigh"].toDouble(90));
        if (props.contains("setpoint")) tw->setSetpoint(props["setpoint"].toDouble(-1));
        return;
    }

    if (typeName == "PumpWidget") {
        auto *pw = static_cast<PumpWidget *>(widget);
        if (props.contains("label")) pw->setLabel(props["label"].toString());
        return;
    }

    if (typeName == "ValveWidget") {
        auto *vw = static_cast<ValveWidget *>(widget);
        if (props.contains("label")) vw->setLabel(props["label"].toString());
        return;
    }

    // PipeWidget_H / PipeWidget_V: no static properties to set
}
