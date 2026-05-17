#ifndef TANK_WIDGET_H
#define TANK_WIDGET_H

#include <QWidget>
#include <QPropertyAnimation>
#include <QTimer>

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
    void setAlarmLimits(qreal low, qreal high) { m_alarmLow = low; m_alarmHigh = high; update(); }

public slots:
    void setLevel(qreal level);
    void setAlarm(bool active);
    void setSetpoint(qreal sp) { m_setpoint = sp; update(); }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    qreal m_level = 0;
    qreal m_targetLevel = 0;
    qreal m_setpoint = -1;
    qreal m_alarmLow = 10, m_alarmHigh = 90;
    bool  m_alarm = false;
    QString m_label;
    QPropertyAnimation *m_anim = nullptr;

    // 表面波动画
    qreal m_surfacePhase = 0;
    QTimer *m_surfaceTimer = nullptr;
    // 报警 LED 闪烁
    bool m_alarmGlowOn = false;
    QTimer *m_alarmGlowTimer = nullptr;
};

#endif
