#include "tag_manager.h"

TagManager::TagManager(QObject *parent) : QObject(parent) {}

void TagManager::registerTag(const QString &name, const QVariant &init)
{
    if (!m_tags.contains(name)) {
        TagValue tv;
        tv.value = init;
        m_tags[name] = tv;
    }
}

void TagManager::updateTag(const QString &name, const QVariant &value, bool valid)
{
    TagValue &tv = m_tags[name];
    tv.value     = value;
    tv.valid     = valid;
    tv.timestamp = QDateTime::currentDateTime();
    emit tagChanged(name, value, valid);
}

QVariant TagManager::tagValue(const QString &name) const
{
    return m_tags.value(name).value;
}

bool TagManager::tagValid(const QString &name) const
{
    return m_tags.contains(name) && m_tags[name].valid;
}
