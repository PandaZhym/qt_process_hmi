#ifndef PUMP_WIDGET_H
#define PUMP_WIDGET_H

#include <QWidget>
#include <QTimer>

class PumpWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal rotation READ rotation WRITE setRotation)

public:
    explicit PumpWidget(QWidget *parent = nullptr);

    bool running() const { return m_running; }
    qreal rotation() const { return m_rotation; }
    qreal speed() const { return m_speed; }

    void setLabel(const QString &label) { m_label = label; update(); }

public slots:
    void setRunning(bool on);
    void setSpeed(qreal pct);
    void setRotation(qreal deg);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    bool m_running = false;
    qreal m_speed = 0;
    qreal m_rotation = 0;
    qreal m_glowPhase = 0;
    QString m_label;
    QTimer *m_animTimer = nullptr;
};

#endif
