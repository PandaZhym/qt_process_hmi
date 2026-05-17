#include "sim_data_manager.h"
#include <QtMath>

SimDataManager::SimDataManager(QObject *parent) : QObject(parent)
{
    m_elapsed.start();
    // initialize all tags
    m_values["TEMP"]  = 50;
    m_values["PRESS"] = 1.5;
    m_values["TANK"]  = 25;
    m_values["FLOW"]  = 15;
    m_values["PUMP"]  = 80;
    m_values["VALVE"] = 50;
}

void SimDataManager::tick()
{
    double t = m_elapsed.elapsed() / 1000.0;

    m_values["TEMP"]  = 50 + 25 * qSin(t * 0.3) + 8 * qSin(t * 0.7 + 1.2);
    m_values["PRESS"] = 1.5 + 0.8 * qSin(t * 0.45 + 0.5) + 0.3 * qSin(t * 1.1);
    m_values["TANK"]  = 25 + 12 * qSin(t * 0.2 + 2.0) + 5 * qSin(t * 0.55);
    m_values["FLOW"]  = 15 + 6 * qSin(t * 0.5 + 1.0) + 3 * qSin(t * 0.85 + 0.8);
    m_values["PUMP"]  = 80 + 15 * qSin(t * 0.35 + 3.0) + 5 * qSin(t * 0.6);
    m_values["VALVE"] = 50 + 25 * qSin(t * 0.15 + 1.5) + 10 * qSin(t * 0.4 + 2.5);

    for (auto it = m_values.begin(); it != m_values.end(); ++it)
        emit valueChanged(it.key(), it.value());
}

double SimDataManager::value(const QString &tagName) const
{
    return m_values.value(tagName, 0.0);
}
