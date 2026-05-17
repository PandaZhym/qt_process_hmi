#ifndef VALUE_DISPLAY_H
#define VALUE_DISPLAY_H

#include <QWidget>
#include <QTimer>

class ValueDisplay : public QWidget
{
    Q_OBJECT

public:
    explicit ValueDisplay(QWidget *parent = nullptr);

    void setLabel(const QString &label) { m_label = label; update(); }
    void setUnit(const QString &unit) { m_unit = unit; update(); }
    void setDecimals(int n) { m_decimals = n; update(); }
    void setAlarmLimits(qreal low, qreal high) { m_alarmLow = low; m_alarmHigh = high; update(); }

public slots:
    void setValue(qreal val);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    qreal m_value = 0;
    qreal m_prevValue = 0;
    QString m_label;
    QString m_unit;
    int m_decimals = 1;
    qreal m_alarmLow = 0, m_alarmHigh = 100;

    // 值变化闪烁
    bool m_flashActive = false;
    QTimer *m_flashTimer = nullptr;
    // 报警脉冲
    bool m_alarmPulse = false;
    QTimer *m_alarmTimer = nullptr;
};

#endif
