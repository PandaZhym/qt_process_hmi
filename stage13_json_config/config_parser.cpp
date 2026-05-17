#include "config_parser.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

ScreenConfig ConfigParser::parse(const QString &jsonPath)
{
    ScreenConfig config;

    QFile file(jsonPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "ConfigParser: cannot open" << jsonPath;
        return config;
    }

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
    if (err.error != QJsonParseError::NoError) {
        qWarning() << "ConfigParser: JSON parse error" << err.errorString();
        return config;
    }

    QJsonObject root = doc.object();
    config.name    = root["name"].toString("Unnamed");
    config.version = root["version"].toInt(0);

    QJsonArray widgetsArr = root["widgets"].toArray();
    for (const auto &wVal : widgetsArr) {
        QJsonObject wObj = wVal.toObject();
        WidgetDef w;
        w.id     = wObj["id"].toString();
        w.type   = wObj["type"].toString();
        w.x      = wObj["x"].toInt(0);
        w.y      = wObj["y"].toInt(0);
        w.width  = wObj["width"].toInt(160);
        w.height = wObj["height"].toInt(120);
        w.properties = wObj["properties"].toObject();

        QJsonArray bindArr = wObj["bindings"].toArray();
        for (const auto &bVal : bindArr) {
            QJsonObject bObj = bVal.toObject();
            WidgetDef::Binding b;
            b.tag       = bObj["tag"].toString();
            b.property  = bObj["property"].toString();
            b.transform = bObj["transform"].toString();
            w.bindings.append(b);
        }

        config.widgets.append(w);
    }

    return config;
}
