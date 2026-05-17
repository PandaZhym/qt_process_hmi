#include "simulated_adapter.h"
#include <QVariantMap>
#include <QtMath>

SimulatedAdapter::SimulatedAdapter(QObject *parent) : IProtocol(parent)
{
    m_connectTimer = new QTimer(this);
    m_connectTimer->setSingleShot(true);
    connect(m_connectTimer, &QTimer::timeout, this, &SimulatedAdapter::onConnectTimer);

    m_pollTimer = new QTimer(this);
    connect(m_pollTimer, &QTimer::timeout, this, &SimulatedAdapter::poll);
}

void SimulatedAdapter::connectTo(const QVariantMap &params)
{
    Q_UNUSED(params);
    // Simulate connection delay (1 second)
    m_connectTimer->start(1000);
}

void SimulatedAdapter::onConnectTimer()
{
    m_connected = true;
    m_elapsed.start();
    emit connected();
}

void SimulatedAdapter::disconnect()
{
    m_pollTimer->stop();
    m_connected = false;
    emit disconnected();
}

void SimulatedAdapter::setMappings(const QVector<TagMapping> &mappings)
{
    m_mappings = mappings;
}

void SimulatedAdapter::startPolling(int intervalMs)
{
    if (m_mappings.isEmpty()) return;
    m_elapsed.start();
    m_pollTimer->start(intervalMs);
}

void SimulatedAdapter::stopPolling()
{
    m_pollTimer->stop();
}

void SimulatedAdapter::writeTag(const QString &tag, const QVariant &value)
{
    m_writtenValues[tag] = value;
}

void SimulatedAdapter::poll()
{
    if (!m_connected) return;

    double t = m_elapsed.elapsed() / 1000.0;

    // Same sine wave formulas as stage10/11
    QHash<QString, QVariant> values;
    values["TEMP"]  = 50 + 25 * qSin(t * 0.3) + 8 * qSin(t * 0.7 + 1.2);
    values["PRESS"] = 1.5 + 0.8 * qSin(t * 0.45 + 0.5) + 0.3 * qSin(t * 1.1);
    values["TANK"]  = 25 + 12 * qSin(t * 0.2 + 2.0) + 5 * qSin(t * 0.55);
    values["FLOW"]  = 15 + 6 * qSin(t * 0.5 + 1.0) + 3 * qSin(t * 0.85 + 0.8);
    values["PUMP"]  = 80 + 15 * qSin(t * 0.35 + 3.0) + 5 * qSin(t * 0.6);
    values["VALVE"] = 50 + 25 * qSin(t * 0.15 + 1.5) + 10 * qSin(t * 0.4 + 2.5);

    // Apply simulated write overrides
    for (auto it = m_writtenValues.begin(); it != m_writtenValues.end(); ++it)
        values[it.key()] = it.value();

    // Filter to only mapped tags
    QHash<QString, QVariant> result;
    for (const auto &m : m_mappings) {
        if (values.contains(m.tagName))
            result[m.tagName] = values[m.tagName];
    }

    emit tagValuesReady(result);
}
