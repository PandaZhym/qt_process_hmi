#include "plc_worker.h"

PlcWorker::PlcWorker(QObject *parent)
    : QObject(parent)
{
    m_client = new TS7Client();
}

PlcWorker::~PlcWorker()
{
    if (m_client->Connected())
        m_client->Disconnect();
    delete m_client;
}

bool PlcWorker::isConnected() const
{
    return m_client->Connected();
}

// ------------------ 连接 / 断开（项目 2 已有）------------------
void PlcWorker::connectToPlc(const QString &ip, int rack, int slot)
{
    QByteArray ipBytes = ip.toUtf8();
    int result = m_client->ConnectTo(ipBytes.constData(), rack, slot);

    if (result == 0) {
        emit connected();
    } else {
        QString msg = QString::fromUtf8(CliErrorText(result).c_str());
        emit errorOccurred(result, msg);
    }
}

void PlcWorker::disconnectFromPlc()
{
    m_client->Disconnect();
    emit disconnected();
}

// ------------------ 读取数据（新增）------------------
void PlcWorker::readArea(int areaType, int dbNumber, int start, int size)
{
    QByteArray buffer(size, '\0');  // 预分配 size 字节的缓冲区
    int result = 0;

    switch (areaType) {
    case AREA_M:
        result = m_client->MBRead(start, size, buffer.data());
        break;
    case AREA_I:
        result = m_client->EBRead(start, size, buffer.data());
        break;
    case AREA_Q:
        result = m_client->ABRead(start, size, buffer.data());
        break;
    case AREA_DB:
        result = m_client->DBRead(dbNumber, start, size, buffer.data());
        break;
    default:
        emit errorOccurred(-1, "未知的区域类型");
        return;
    }

    if (result == 0) {
        emit dataRead(areaType, dbNumber, start, buffer);
    } else {
        QString msg = QString::fromUtf8(CliErrorText(result).c_str());
        emit errorOccurred(result, msg);
    }
}
