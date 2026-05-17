#ifndef TANK_WIDGET_H
#define TANK_WIDGET_H

#include <QWidget>
#include <QPropertyAnimation>
#include <QTimer>

class TankWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal level READ level WRITE setLevel)
public:
    explicit TankWidget(QWidget *parent = nullptr);
    qreal level() const { return m_level; }
    void setLabel(const QString &label) { m_label = label; update(); }
    void setAlarmLimits(qreal low, qreal high) { m_alarmLow = low; m_alarmHigh = high; update(); }
public slots:
    void setLevel(qreal level);
    void setSetpoint(qreal sp) { m_setpoint = sp; update(); }
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    qreal m_level = 0, m_setpoint = -1;
    qreal m_alarmLow = 10, m_alarmHigh = 90;
    QString m_label;
    QPropertyAnimation *m_anim = nullptr;
    // 表面波动画
    qreal m_surfacePhase = 0;
    QTimer *m_surfaceTimer = nullptr;
};
#endif
