#ifndef VALVE_WIDGET_H
#define VALVE_WIDGET_H

#include <QWidget>
#include <QPropertyAnimation>

class ValveWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal opening READ opening WRITE setOpening)
public:
    explicit ValveWidget(QWidget *parent = nullptr);
    qreal opening() const { return m_opening; }
    void setLabel(const QString &label) { m_label = label; update(); }
public slots:
    void setOpening(qreal pct);
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    qreal m_opening = 0;
    QString m_label;
    QPropertyAnimation *m_anim = nullptr;
};
#endif
