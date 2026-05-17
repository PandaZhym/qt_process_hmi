#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

#include <QString>
#include <QVector>
#include <QJsonObject>

struct WidgetDef {
    QString id;
    QString type;
    int     x      = 0;
    int     y      = 0;
    int     width  = 160;
    int     height = 120;

    QJsonObject properties;

    struct Binding {
        QString tag;
        QString property;
        QString transform;       // empty = pass-through
    };
    QVector<Binding> bindings;
};

struct ScreenConfig {
    QString name;
    int     version = 0;
    QVector<WidgetDef> widgets;
};

class ConfigParser
{
public:
    static ScreenConfig parse(const QString &jsonPath);

private:
    ConfigParser() = default;
};

#endif
