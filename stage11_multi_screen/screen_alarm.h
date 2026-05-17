#ifndef SCREEN_ALARM_H
#define SCREEN_ALARM_H

#include "hmi_screen.h"
#include <QVector>
#include <QDateTime>
#include <QLabel>
#include <QPushButton>
#include <QTimer>

class ScreenAlarm : public HmiScreen
{
    Q_OBJECT
public:
    explicit ScreenAlarm(QWidget *parent = nullptr);

    void onEnter() override;
    void onTick() override;

    int activeCount() const { return m_activeCount; }
    int unackedCount() const { return m_unackedCount; }

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void ackAll();

private:
    struct AlarmEntry {
        int id;
        QString tagName;
        QString typeStr;
        double  limit;
        double  actualValue;
        QDateTime timestamp;
        bool acknowledged = false;
        bool active       = true;
    };

    struct Threshold {
        double hh, h, l, ll;
    };

    void    checkTag(const QString &tag, double value,
                     const Threshold &thresh);
    void    fireAlarm(const QString &tag, const QString &type,
                      double limit, double value);

    QVector<AlarmEntry> m_alarms;
    int m_nextId      = 1;
    int m_activeCount  = 0;
    int m_unackedCount = 0;

    QHash<QString, QStringList> m_activeKeys;  // tag -> active alarm types

    // Header widgets (not self-painted)
    QLabel      *m_headerLabel = nullptr;
    QPushButton *m_ackAllBtn   = nullptr;

    // Blink
    QTimer *m_blinkTimer = nullptr;
    int     m_blinkPhase = 0;

    static constexpr int ROW_HEIGHT = 30;
    static constexpr int PADDING    = 6;
};

#endif
