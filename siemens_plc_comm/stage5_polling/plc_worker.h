#pragma once
#include <QObject>
#include <QTimer>
#include <QVector>
#include "snap7.h"

#define AREA_M   0
#define AREA_I   1
#define AREA_Q   2
#define AREA_DB  3

// 监控项：描述"读取哪个地址、按什么类型解析"
struct MonitorItem {
    int area   = AREA_M;
    int dbNum  = 0;
    int start  = 0;
    int valType = 4;    // 默认 UINT16
};

class PlcWorker : public QObject
{
    Q_OBJECT

public:
    explicit PlcWorker(QObject *parent = nullptr);
    ~PlcWorker();

public slots:
    void connectToPlc(const QString &ip, int rack, int slot);
    void disconnectFromPlc();
    void readArea(int areaType, int dbNumber, int start, int size);
    void writeArea(int areaType, int dbNumber, int start,
                   const QByteArray &data);

    // 新增：轮询控制
    void startPolling(int intervalMs);
    void stopPolling();
    void setMonitorItems(const QVector<MonitorItem> &items);

signals:
    void connected();
    void disconnected();
    void errorOccurred(int code, const QString &message);
    void dataRead(int areaType, int dbNumber, int start,
                  const QByteArray &data);
    void dataWritten(int areaType, int dbNumber, int start);

    // 新增：轮询信号
    void pollingTick();  // 一次轮询完成
    // 第 idx 个监控项的数据
    void pollingData(int idx, int area, int dbNum, int start,
                     const QByteArray &data);
    void cpuStatusChanged(int status);  // S7CpuStatusRun / S7CpuStatusStop

private:
    void pollAllItems();
    void checkCpuStatus();
    int  sizeForType(int valType) const;

    TS7Client *m_client;
    QTimer    *m_timer;     // 轮询定时器（在 worker 线程中）

    // 自动重连参数
    QString m_lastIp;
    int     m_lastRack = 0;
    int     m_lastSlot = 1;
    int     m_consecutiveErrors = 0;  // 连续读失败次数
    bool    m_wasConnected = false;    // 之前是否成功连上过
    bool    m_reconnecting = false;    // 正在尝试重连，避免每轮都调 ConnectTo

    // 监控项列表
    QVector<MonitorItem> m_items;
};
