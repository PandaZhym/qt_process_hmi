#include "plc_worker.h"

PlcWorker::PlcWorker(QObject *parent) : QObject(parent)
{
    m_client = new TS7Client();
    m_timer  = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &PlcWorker::pollAllItems);
}

PlcWorker::~PlcWorker()
{
    m_timer->stop();
    if (m_client->Connected())
        m_client->Disconnect();
    delete m_client;
}

void PlcWorker::connectToPlc(const QString &ip, int rack, int slot)
{
    m_lastIp   = ip;
    m_lastRack = rack;
    m_lastSlot = slot;

    QByteArray ipBytes = ip.toUtf8();
    int result = m_client->ConnectTo(ipBytes.constData(), rack, slot);
    if (result == 0)
        emit connected();
    else
        emit errorOccurred(result, QString::fromUtf8(CliErrorText(result).c_str()));
}

void PlcWorker::disconnectFromPlc()
{
    m_timer->stop();
    m_client->Disconnect();
    emit disconnected();
}

void PlcWorker::setDataBlock(int dbNum)
{
    m_dbNum = dbNum;
}

void PlcWorker::setPollInterval(int ms)
{
    if (ms > 0)
        m_timer->start(ms);
    else
        m_timer->stop();
}

void PlcWorker::pollAllItems()
{
    if (!m_client->Connected()) {
        if (!m_lastIp.isEmpty()) {
            QByteArray ipBytes = m_lastIp.toUtf8();
            int ret = m_client->ConnectTo(ipBytes.constData(), m_lastRack, m_lastSlot);
            if (ret != 0) {
                emit disconnected();
                return;
            }
        } else {
            return;
        }
    }

    checkCpuStatus();

    float tankLevel, pumpSpeed, temp, pressure, flow;
    bool pumpRun;
    float valveOpen;

    bool ok = true;
    ok = ok && (readFloatFromDB(m_dbNum, 0,  tankLevel) == 0);
    ok = ok && (readFloatFromDB(m_dbNum, 4,  pumpSpeed)  == 0);
    ok = ok && (readBoolFromDB(m_dbNum,  24, 0, pumpRun) == 0);
    ok = ok && (readFloatFromDB(m_dbNum, 8,  valveOpen)  == 0);
    ok = ok && (readFloatFromDB(m_dbNum, 12, temp)       == 0);
    ok = ok && (readFloatFromDB(m_dbNum, 16, pressure)   == 0);
    ok = ok && (readFloatFromDB(m_dbNum, 20, flow)       == 0);

    if (ok)
        emit processDataUpdated(tankLevel, pumpSpeed, pumpRun,
                                valveOpen, temp, pressure, flow);
}

int PlcWorker::readFloatFromDB(int dbNum, int start, float &out)
{
    QByteArray buf(4, '\0');
    int ret = m_client->DBRead(dbNum, start, 4, buf.data());
    if (ret == 0) {
        quint32 bits = ((quint8)buf[0] << 24) | ((quint8)buf[1] << 16)
                     | ((quint8)buf[2] << 8)  | (quint8)buf[3];
        std::memcpy(&out, &bits, 4);
    }
    return ret;
}

int PlcWorker::readBoolFromDB(int dbNum, int byteOff, int bitOff, bool &out)
{
    QByteArray buf(1, '\0');
    int ret = m_client->DBRead(dbNum, byteOff, 1, buf.data());
    if (ret == 0)
        out = ((quint8)buf[0] >> bitOff) & 0x01;
    return ret;
}

void PlcWorker::checkCpuStatus()
{
    int status = m_client->PlcStatus();
    emit cpuStatusChanged(status);
}
