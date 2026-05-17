#ifndef ALARM_BANNER_H
#define ALARM_BANNER_H

#include <QWidget>
#include <QTimer>
#include "alarm_manager.h"

class AlarmBanner : public QWidget
{
    Q_OBJECT

public:
    explicit AlarmBanner(QWidget *parent = nullptr);

    void updateState(int activeCount, int unackedCount,
                     const AlarmRecord *latestUnacked = nullptr);

signals:
    void clicked();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    int m_activeCount = 0;
    int m_unackedCount = 0;
    QString m_latestMsg;
    qreal m_pulsePhase = 0;
    QTimer *m_pulseTimer = nullptr;
};

#endif
