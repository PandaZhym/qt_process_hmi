#ifndef SIM_DATA_MANAGER_H
#define SIM_DATA_MANAGER_H

#include <QObject>
#include <QHash>
#include <QString>
#include <QElapsedTimer>

class SimDataManager : public QObject
{
    Q_OBJECT
public:
    explicit SimDataManager(QObject *parent = nullptr);

    void tick();

    double value(const QString &tagName) const;
    QStringList tagNames() const { return m_values.keys(); }

signals:
    void valueChanged(const QString &tagName, double value);

private:
    QHash<QString, double> m_values;
    QElapsedTimer m_elapsed;
};

#endif
