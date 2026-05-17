#pragma once
#include <QObject>
#include "snap7.h"

#define AREA_M   0
#define AREA_I   1
#define AREA_Q   2
#define AREA_DB  3

class PlcWorker : public QObject
{
    Q_OBJECT

public:
    explicit PlcWorker(QObject *parent = nullptr);
    ~PlcWorker();

    bool isConnected() const;

public slots:
    void connectToPlc(const QString &ip, int rack, int slot);
    void disconnectFromPlc();
    void readArea(int areaType, int dbNumber, int start, int size);
    // 新增：写入数据（data 已由调用方打包成大端字节序）
    void writeArea(int areaType, int dbNumber, int start,
                   const QByteArray &data);

signals:
    void connected();
    void disconnected();
    void errorOccurred(int code, const QString &message);
    void dataRead(int areaType, int dbNumber, int start,
                  const QByteArray &data);
    // 新增：写入完成信号
    void dataWritten(int areaType, int dbNumber, int start);

private:
    TS7Client *m_client;
};
