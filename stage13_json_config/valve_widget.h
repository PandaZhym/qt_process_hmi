#ifndef VALVE_WIDGET_H
#define VALVE_WIDGET_H

#include <QWidget>
#include <QPropertyAnimation>

// 阀门控件 -- 三角形开度指示，0=关闭, 100=全开
class ValveWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal opening READ opening WRITE setOpening)

public:
    explicit ValveWidget(QWidget *parent = nullptr);

    qreal opening() const { return m_opening; }

    void setLabel(const QString &label) { m_label = label; update(); }

public slots:
    void setOpening(qreal pct);        // 0.0 - 100.0

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    qreal m_opening = 0;
    qreal m_targetOpening = 0;
    QString m_label;
};

#endif
