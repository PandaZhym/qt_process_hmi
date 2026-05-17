#ifndef I_PROTOCOL_H
#define I_PROTOCOL_H

#include <QObject>
#include <QHash>
#include <QVector>
#include <QVariantMap>
#include <QVariant>
#include "tag_mapping.h"

class IProtocol : public QObject
{
    Q_OBJECT
public:
    explicit IProtocol(QObject *parent = nullptr) : QObject(parent) {}

    virtual bool    isConnected() const = 0;
    virtual QString protocolName() const = 0;

public slots:
    virtual void connectTo(const QVariantMap &params) = 0;
    virtual void disconnect() = 0;
    virtual void setMappings(const QVector<TagMapping> &mappings) = 0;
    virtual void startPolling(int intervalMs = 500) = 0;
    virtual void stopPolling() = 0;
    virtual void writeTag(const QString &tag, const QVariant &value) = 0;

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString &msg);
    void tagValuesReady(const QHash<QString, QVariant> &values);
};

#endif
