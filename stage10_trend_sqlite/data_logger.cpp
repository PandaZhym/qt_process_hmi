#include "data_logger.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QTimer>
#include <QDebug>

DataLogger::DataLogger(const QString &dbPath, QObject *parent)
    : QObject(parent), m_dbPath(dbPath) {}

DataLogger::~DataLogger() { stop(); }

void DataLogger::start()
{
    m_workerThread = new QThread(this);
    moveToThread(m_workerThread);

    connect(m_workerThread, &QThread::started, this, [this]() {
        // 在工作线程中初始化数据库连接
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "logger_conn");
        db.setDatabaseName(m_dbPath);
        if (!db.open()) {
            emit error("无法打开数据库: " + db.lastError().text());
            return;
        }
        ensureTable();

        // 定期 flush 定时器
        QTimer *flushTimer = new QTimer(this);
        connect(flushTimer, &QTimer::timeout, this, &DataLogger::flushNow);
        flushTimer->start(500);

        m_running = true;
        emit started();
    });

    m_workerThread->start();
}

void DataLogger::stop()
{
    if (m_workerThread) {
        m_workerThread->quit();
        m_workerThread->wait(3000);
        m_workerThread->deleteLater();
        m_workerThread = nullptr;
    }
    m_running = false;
}

void DataLogger::logValue(const QString &tagName, double value, qint64 timestamp)
{
    if (!m_running) return;

    QMutexLocker lock(&m_mutex);
    m_buffer.append({tagName, value, timestamp});
    // 缓冲区满 1000 条，立即触发 flush
    if (m_buffer.size() >= 1000) {
        lock.unlock();
        flushNow();
    }
}

void DataLogger::flushNow()
{
    QVector<DataPoint> batch;
    {
        QMutexLocker lock(&m_mutex);
        if (m_buffer.isEmpty()) return;
        batch.swap(m_buffer);
    }

    QSqlDatabase db = QSqlDatabase::database("logger_conn");
    if (!db.isOpen()) return;

    QSqlQuery query(db);
    query.prepare("INSERT INTO tag_history (tag_name, value, timestamp) "
                  "VALUES (?, ?, ?)");
    db.transaction();
    for (const auto &dp : batch) {
        query.addBindValue(dp.tagName);
        query.addBindValue(dp.value);
        query.addBindValue(dp.timestamp);
        if (!query.exec()) {
            qWarning() << "DataLogger insert error:" << query.lastError().text();
        }
    }
    db.commit();
}

void DataLogger::queryHistory(const QString &tagName, qint64 fromTime, qint64 toTime)
{
    QSqlDatabase db = QSqlDatabase::database("logger_conn");
    if (!db.isOpen()) return;

    QSqlQuery q(db);
    q.prepare("SELECT timestamp, value FROM tag_history "
              "WHERE tag_name = ? AND timestamp BETWEEN ? AND ? "
              "ORDER BY timestamp ASC");
    q.addBindValue(tagName);
    q.addBindValue(fromTime);
    q.addBindValue(toTime);

    QVector<QPair<qint64, double>> result;
    if (q.exec()) {
        while (q.next()) {
            result.append({q.value(0).toLongLong(), q.value(1).toDouble()});
        }
    }
    emit queryResult(tagName, result);
}

void DataLogger::ensureTable()
{
    QSqlDatabase db = QSqlDatabase::database("logger_conn");
    QSqlQuery q(db);
    q.exec("CREATE TABLE IF NOT EXISTS tag_history ("
           "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
           "  tag_name TEXT NOT NULL,"
           "  value REAL NOT NULL,"
           "  timestamp INTEGER NOT NULL"
           ")");
    q.exec("CREATE INDEX IF NOT EXISTS idx_tag_time ON tag_history(tag_name, timestamp)");
}
