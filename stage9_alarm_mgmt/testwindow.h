#ifndef TESTWINDOW_H
#define TESTWINDOW_H

#include <QWidget>
#include <QTimer>
#include <QPushButton>
#include "alarm_manager.h"
#include "alarm_list_widget.h"
#include "alarm_banner.h"
#include "tag_display.h"

class TestWindow : public QWidget
{
    Q_OBJECT

public:
    explicit TestWindow(QWidget *parent = nullptr);

private slots:
    void tick();
    void onAlarmTriggered(const AlarmRecord &rec);
    void onCountsChanged(int active, int unacked);
    void acknowledgeAll();

private:
    void setupUi();
    void applyTheme();
    void setupAlarmConditions();

    AlarmManager    *m_alarmMan = nullptr;
    AlarmListWidget *m_listWidget = nullptr;
    AlarmBanner     *m_banner = nullptr;
    QPushButton     *m_ackBtn = nullptr;

    // 6 个标签显示
    TagDisplay *m_dispTank  = nullptr;
    TagDisplay *m_dispPump  = nullptr;
    TagDisplay *m_dispTemp  = nullptr;
    TagDisplay *m_dispPress = nullptr;
    TagDisplay *m_dispFlow  = nullptr;
    TagDisplay *m_dispValve = nullptr;

    QTimer *m_simTimer = nullptr;
    qreal m_simPhase = 0;

    // 跟踪最后一次未确认报警
    AlarmRecord m_lastUnacked;
    bool m_hasLastUnacked = false;
};

#endif
