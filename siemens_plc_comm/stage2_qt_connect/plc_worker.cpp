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

void PlcWorker::connectToPlc(const QString &ip, int rack, int slot)
{
    // 转成 C 字符串（snap7 需要 const char*）
    QByteArray ipBytes = ip.toUtf8();
    int result = m_client->ConnectTo(ipBytes.constData(), rack, slot);

    if (result == 0) {
        emit connected();
    } else {
        QString errMsg = QString::fromUtf8(CliErrorText(result).c_str());
        emit errorOccurred(result, errMsg);
    }
}

void PlcWorker::disconnectFromPlc()
{
    m_client->Disconnect();
    emit disconnected();
}
