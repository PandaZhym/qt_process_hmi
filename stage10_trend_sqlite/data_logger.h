#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#include <QObject>
#include <QThread>
#include <QVector>
#include <QMutex>
#include <QPair>

struct DataPoint {
    QString tagName;
    double  value = 0;
    qint64  timestamp = 0;   // ms since epoch
};

class DataLogger : public QObject
{
    Q_OBJECT

public:
    explicit DataLogger(const QString &dbPath, QObject *parent = nullptr);
    ~DataLogger() override;

    void start();
    void stop();

public slots:
    void logValue(const QString &tagName, double value, qint64 timestamp);
    void flushNow();
    void queryHistory(const QString &tagName, qint64 fromTime, qint64 toTime);

signals:
    void queryResult(const QString &tagName, const QVector<QPair<qint64, double>> &data);
    void started();
    void error(const QString &msg);

private:
    void ensureTable();

    QString m_dbPath;
    QThread *m_workerThread = nullptr;
    QVector<DataPoint> m_buffer;
    QMutex m_mutex;
    bool m_running = false;
};

#endif
