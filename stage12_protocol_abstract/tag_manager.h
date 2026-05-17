#ifndef TAG_MANAGER_H
#define TAG_MANAGER_H

#include <QObject>
#include <QHash>
#include <QVariant>
#include <QDateTime>

struct TagValue {
    QVariant  value;
    bool      valid = false;
    QDateTime timestamp;
};

class TagManager : public QObject
{
    Q_OBJECT

public:
    explicit TagManager(QObject *parent = nullptr);

    void updateTag(const QString &name, const QVariant &value, bool valid = true);

    QVariant tagValue(const QString &name) const;
    bool     tagValid(const QString &name) const;

    void registerTag(const QString &name, const QVariant &init = QVariant());

signals:
    void tagChanged(const QString &name, const QVariant &value, bool valid);

private:
    QHash<QString, TagValue> m_tags;
};

#endif
