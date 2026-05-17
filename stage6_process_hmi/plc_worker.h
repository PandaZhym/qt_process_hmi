#ifndef PLC_WORKER_H
#define PLC_WORKER_H

#include <QObject>
#include <QTimer>
#include <QByteArray>
#include "snap7.h"

class PlcWorker : public QObject
{
    Q_OBJECT

public:
    explicit PlcWorker(QObject *parent = nullptr);
    ~PlcWorker() override;

    bool isConnected() const { return m_client->Connected(); }

public slots:
    void connectToPlc(const QString &ip, int rack, int slot);
    void disconnectFromPlc();
    void setDataBlock(int dbNum);           // 设置读取的 DB 号
    void setPollInterval(int ms);

signals:
    void connected();
    void disconnected();
    void errorOccurred(int code, const QString &msg);

    // 实时数据信号（解析后的值）
    void processDataUpdated(qreal tankLevel, qreal pumpSpeed, bool pumpRun,
                            qreal valveOpen, qreal temperature,
                            qreal pressure, qreal flowRate);
    void cpuStatusChanged(int status);

private:
    void pollAllItems();
    int readFloatFromDB(int dbNum, int start, float &out);
    int readBoolFromDB(int dbNum, int byteOff, int bitOff, bool &out);
    void checkCpuStatus();

    TS7Client *m_client = nullptr;
    QTimer *m_timer = nullptr;
    QString m_lastIp;
    int m_lastRack = 0, m_lastSlot = 1;
    int m_dbNum = 1;
};

#endif
