#ifndef TAG_MANAGER_H
#define TAG_MANAGER_H

#include <QObject>
#include <QHash>
#include <QVariant>
#include <QDateTime>

struct TagValue {
    QVariant value;
    bool     valid = false;
    QDateTime timestamp;
};

class TagManager : public QObject
{
    Q_OBJECT

public:
    explicit TagManager(QObject *parent = nullptr);

    // 通讯层调用：更新标签值（跨线程安全，信号在接收者线程触发）
    void updateTag(const QString &name, const QVariant &value, bool valid = true);

    // 画面层调用：读取当前值
    QVariant tagValue(const QString &name) const;
    bool tagValid(const QString &name) const;

    // 批量注册（可选，预分配使查找更快）
    void registerTag(const QString &name, const QVariant &init = QVariant());

signals:
    // 标签值变化（画面层连接此信号自动刷新）
    void tagChanged(const QString &name, const QVariant &value, bool valid);

private:
    QHash<QString, TagValue> m_tags;
};

#endif
