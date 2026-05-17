#pragma once
#include <QObject>
#include "snap7.h"

// PlcWorker：在子线程中执行 PLC 操作的 QObject
// 将阻塞的 snap7 调用与 UI 线程隔离开
class PlcWorker : public QObject
{
    Q_OBJECT  // Qt 元对象宏——让 QObject 支持信号/槽

public:
    explicit PlcWorker(QObject *parent = nullptr);
    ~PlcWorker();

public slots:
    // 这些槽函数通过跨线程信号触发，在 worker 线程中执行
    void connectToPlc(const QString &ip, int rack, int slot);
    void disconnectFromPlc();

signals:
    // 这些信号发回 UI 线程
    void connected();
    void disconnected();
    void errorOccurred(int code, const QString &message);

private:
    TS7Client *m_client;
};
