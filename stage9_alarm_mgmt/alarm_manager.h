#ifndef ALARM_MANAGER_H
#define ALARM_MANAGER_H

#include <QObject>
#include <QVector>
#include <QDateTime>
#include <QHash>
#include <QSet>
#include <cmath>

// 报警类型
enum class AlarmType {
    HH = 0,  // 高高报
    H  = 1,  // 高报
    L  = 2,  // 低报
    LL = 3   // 低低报
};

inline const char *alarmTypeName(AlarmType t) {
    switch (t) {
        case AlarmType::HH: return "HH";
        case AlarmType::H:  return "H";
        case AlarmType::L:  return "L";
        case AlarmType::LL: return "LL";
    }
    return "";
}

// 报警条件配置
struct AlarmCondition {
    QString   tagName;
    double    hhLimit = NAN;
    double    hLimit  = NAN;
    double    lLimit  = NAN;
    double    llLimit = NAN;
    bool      enabled = true;

    bool hasHH() const { return !std::isnan(hhLimit); }
    bool hasH()  const { return !std::isnan(hLimit); }
    bool hasL()  const { return !std::isnan(lLimit); }
    bool hasLL() const { return !std::isnan(llLimit); }
};

// 单条报警记录
struct AlarmRecord {
    int        id       = 0;
    QString    tagName;
    AlarmType  type     = AlarmType::H;
    double     limit    = 0;
    double     actualValue = 0;
    QDateTime  timestamp;
    bool       acknowledged = false;
    bool       active    = true;   // false = 已恢复正常
};

class AlarmManager : public QObject
{
    Q_OBJECT

public:
    explicit AlarmManager(QObject *parent = nullptr);

    void registerCondition(const AlarmCondition &cond);
    void evaluateValue(const QString &tagName, double value);

    void acknowledgeAll();
    void acknowledgeId(int id);
    const QVector<AlarmRecord> &records() const { return m_records; }
    int unacknowledgedCount() const { return m_unackedCount; }
    int activeCount() const { return m_activeCount; }

signals:
    void alarmTriggered(const AlarmRecord &record);
    void alarmCleared(int id);
    void alarmAcknowledged(int id);
    void countsChanged(int active, int unacked);

private:
    static constexpr double HYSTERESIS = 0.02;  // 2% 回差防抖
    void checkCondition(const AlarmCondition &cond, double value,
                        AlarmType type, double limit);
    void fireAlarm(const QString &tagName, AlarmType type,
                   double limit, double value);

    QVector<AlarmCondition> m_conditions;
    QVector<AlarmRecord>    m_records;
    int m_nextId = 1;
    int m_unackedCount = 0;
    int m_activeCount  = 0;
    QHash<QString, QSet<AlarmType>> m_activeKeys; // 防重复触发
};

#endif
