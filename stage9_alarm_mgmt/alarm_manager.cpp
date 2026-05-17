#include "alarm_manager.h"

AlarmManager::AlarmManager(QObject *parent) : QObject(parent) {}

void AlarmManager::registerCondition(const AlarmCondition &cond)
{
    if (!cond.enabled) return;
    m_conditions.append(cond);
}

void AlarmManager::evaluateValue(const QString &tagName, double value)
{
    for (const auto &cond : m_conditions) {
        if (cond.tagName != tagName) continue;

        if (cond.hasHH()) checkCondition(cond, value, AlarmType::HH, cond.hhLimit);
        if (cond.hasH())  checkCondition(cond, value, AlarmType::H,  cond.hLimit);
        if (cond.hasL())  checkCondition(cond, value, AlarmType::L,  cond.lLimit);
        if (cond.hasLL()) checkCondition(cond, value, AlarmType::LL, cond.llLimit);
    }
}

void AlarmManager::checkCondition(const AlarmCondition &cond, double value,
                                   AlarmType type, double limit)
{
    QString key = cond.tagName + "_" + QString::number((int)type);
    bool isActive = m_activeKeys[cond.tagName].contains(type);

    bool shouldFire = false;
    bool shouldClear = false;

    if (type == AlarmType::HH || type == AlarmType::H) {
        // 高报：超过限制触发
        if (!isActive && value > limit) shouldFire = true;
        // 回差：降到限制的 98% 才清除
        else if (isActive && value < limit * (1.0 - HYSTERESIS)) shouldClear = true;
    } else {
        // 低报：低于限制触发
        if (!isActive && value < limit) shouldFire = true;
        // 回差：升到限制的 102% 才清除
        else if (isActive && value > limit * (1.0 + HYSTERESIS)) shouldClear = true;
    }

    if (shouldFire) {
        fireAlarm(cond.tagName, type, limit, value);
        m_activeKeys[cond.tagName].insert(type);
    }

    if (shouldClear) {
        // 标记活跃报警为已清除（但未确认状态保留，仍需操作员确认）
        for (auto &rec : m_records) {
            if (rec.tagName == cond.tagName && rec.type == type && rec.active) {
                rec.active = false;
                m_activeCount--;
                // 注意：不清除 m_unackedCount，已恢复但未确认的报警仍需确认
                emit alarmCleared(rec.id);
            }
        }
        m_activeKeys[cond.tagName].remove(type);
        emit countsChanged(m_activeCount, m_unackedCount);
    }
}

void AlarmManager::fireAlarm(const QString &tagName, AlarmType type,
                              double limit, double value)
{
    AlarmRecord rec;
    rec.id          = m_nextId++;
    rec.tagName     = tagName;
    rec.type        = type;
    rec.limit       = limit;
    rec.actualValue = value;
    rec.timestamp   = QDateTime::currentDateTime();
    rec.acknowledged = false;
    rec.active      = true;

    m_records.append(rec);
    m_activeCount++;
    m_unackedCount++;

    emit alarmTriggered(rec);
    emit countsChanged(m_activeCount, m_unackedCount);
}

void AlarmManager::acknowledgeAll()
{
    for (auto &rec : m_records) {
        if (!rec.acknowledged) {
            rec.acknowledged = true;
            m_unackedCount--;
            emit alarmAcknowledged(rec.id);
        }
    }
    if (m_unackedCount < 0) m_unackedCount = 0;
    emit countsChanged(m_activeCount, m_unackedCount);
}

void AlarmManager::acknowledgeId(int id)
{
    for (auto &rec : m_records) {
        if (rec.id == id && !rec.acknowledged) {
            rec.acknowledged = true;
            m_unackedCount--;
            emit alarmAcknowledged(rec.id);
            break;
        }
    }
    emit countsChanged(m_activeCount, m_unackedCount);
}
