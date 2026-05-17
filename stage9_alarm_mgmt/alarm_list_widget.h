#ifndef ALARM_LIST_WIDGET_H
#define ALARM_LIST_WIDGET_H

#include <QWidget>
#include <QTimer>
#include <QVector>
#include "alarm_manager.h"

class AlarmListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AlarmListWidget(QWidget *parent = nullptr);

    void setAlarmRecords(const QVector<AlarmRecord> &records);

signals:
    void acknowledgeRequested(int id);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    QVector<AlarmRecord> m_records;
    qreal m_blinkPhase = 0;
    QTimer *m_blinkTimer = nullptr;
    int m_hoveredRow = -1;

    static constexpr int ROW_HEIGHT = 36;
    static constexpr int PADDING = 6;
};

#endif
