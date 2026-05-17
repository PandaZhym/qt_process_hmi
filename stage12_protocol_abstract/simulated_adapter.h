#ifndef SIMULATED_ADAPTER_H
#define SIMULATED_ADAPTER_H

#include "i_protocol.h"
#include <QTimer>
#include <QElapsedTimer>

class SimulatedAdapter : public IProtocol
{
    Q_OBJECT
public:
    explicit SimulatedAdapter(QObject *parent = nullptr);

    bool    isConnected() const override { return m_connected; }
    QString protocolName() const override { return "SIM"; }

public slots:
    void connectTo(const QVariantMap &params) override;
    void disconnect() override;
    void setMappings(const QVector<TagMapping> &mappings) override;
    void startPolling(int intervalMs = 500) override;
    void stopPolling() override;
    void writeTag(const QString &tag, const QVariant &value) override;

private slots:
    void onConnectTimer();
    void poll();

private:
    bool    m_connected = false;
    QTimer *m_connectTimer = nullptr;
    QTimer *m_pollTimer    = nullptr;
    QElapsedTimer m_elapsed;

    QVector<TagMapping> m_mappings;
    QHash<QString, QVariant> m_writtenValues;  // simulated write target
};

#endif
