#ifndef TANK_WIDGET_H
#define TANK_WIDGET_H

#include <QWidget>
#include <QPropertyAnimation>

// 罐子控件 -- 显示液位百分比，带颜色渐变和动画
class TankWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal level READ level WRITE setLevel)
    Q_PROPERTY(bool alarm READ alarm WRITE setAlarm)

public:
    explicit TankWidget(QWidget *parent = nullptr);

    qreal level() const { return m_level; }
    bool alarm() const { return m_alarm; }

    void setLabel(const QString &label) { m_label = label; update(); }
    void setAlarmLimits(qreal low, qreal high) { m_alarmLow = low; m_alarmHigh = high; }

public slots:
    void setLevel(qreal level);       // 0.0 - 100.0
    void setAlarm(bool active);
    void setSetpoint(qreal sp) { m_setpoint = sp; update(); }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    qreal m_level = 0;
    qreal m_targetLevel = 0;
    qreal m_setpoint = -1;            // -1 = 不显示
    qreal m_alarmLow = 10, m_alarmHigh = 90;
    bool  m_alarm = false;
    QString m_label;
    QPropertyAnimation *m_anim = nullptr;
};

#endif
