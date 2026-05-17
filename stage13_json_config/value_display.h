#ifndef VALUE_DISPLAY_H
#define VALUE_DISPLAY_H

#include <QWidget>

// 数值显示控件 -- 大号数值 + 标签 + 单位 + 报警边框
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
    QString m_label;
    QString m_unit;
    int m_decimals = 1;
    qreal m_alarmLow = 0, m_alarmHigh = 100;
    bool m_connected = false;
};

#endif
