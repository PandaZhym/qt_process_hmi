#pragma once
#include <QObject>
#include "snap7.h"

// PLC 存储区域代号
#define AREA_M   0   // M 区（Merker，中间继电器）
#define AREA_I   1   // I 区（Input，输入映像，snap7 叫 EB）
#define AREA_Q   2   // Q 区（Output，输出映像，snap7 叫 AB）
#define AREA_DB  3   // DB 块（Data Block）

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

signals:
    void connected();
    void disconnected();
    void errorOccurred(int code, const QString &message);
    // 读取成功时发出，data 为原始字节
    void dataRead(int areaType, int dbNumber, int start, const QByteArray &data);

private:
    TS7Client *m_client;
};
