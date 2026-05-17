#ifndef WIDGET_FACTORY_H
#define WIDGET_FACTORY_H

#include <QWidget>
#include <QHash>
#include <QJsonObject>
#include <functional>

class WidgetFactory
{
public:
    using CreatorFunc = std::function<QWidget*(QWidget *parent)>;

    static QWidget *create(const QString &typeName, QWidget *parent = nullptr);
    static void     applyProperties(const QString &typeName, QWidget *widget,
                                    const QJsonObject &props);

private:
    WidgetFactory() = default;

    static QHash<QString, CreatorFunc> &creators();
    static void ensureInit();
};

#endif
