#include "plc_worker.h"

PlcWorker::PlcWorker(QObject *parent)
    : QObject(parent)
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

// ------------------ 连接 / 断开 ------------------
void PlcWorker::connectToPlc(const QString &ip, int rack, int slot)
{
    m_lastIp   = ip;
    m_lastRack = rack;
    m_lastSlot = slot;

    QByteArray ipBytes = ip.toUtf8();
    int result = m_client->ConnectTo(ipBytes.constData(), rack, slot);

    if (result == 0) {
        m_wasConnected = true;
        emit connected();
    } else {
        QString msg = QString::fromUtf8(CliErrorText(result).c_str());
        emit errorOccurred(result, msg);
    }
}

void PlcWorker::disconnectFromPlc()
{
    m_wasConnected = false;
    m_timer->stop();
    m_client->Disconnect();
    emit disconnected();
}

// ------------------ 读写（同项目4）------------------
void PlcWorker::readArea(int areaType, int dbNumber, int start, int size)
{
    QByteArray buffer(size, '\0');
    int result = 0;
    switch (areaType) {
    case AREA_M:  result = m_client->MBRead(start, size, buffer.data()); break;
    case AREA_I:  result = m_client->EBRead(start, size, buffer.data()); break;
    case AREA_Q:  result = m_client->ABRead(start, size, buffer.data()); break;
    case AREA_DB: result = m_client->DBRead(dbNumber, start, size, buffer.data()); break;
    default: return;
    }
    if (result == 0)
        emit dataRead(areaType, dbNumber, start, buffer);
    else {
        QString msg = QString::fromUtf8(CliErrorText(result).c_str());
        emit errorOccurred(result, msg);
    }
}

void PlcWorker::writeArea(int areaType, int dbNumber, int start,
                          const QByteArray &data)
{
    int result = 0;
    switch (areaType) {
    case AREA_M:  result = m_client->MBWrite(start, data.size(), (void *)data.constData()); break;
    case AREA_I:  result = m_client->EBWrite(start, data.size(), (void *)data.constData()); break;
    case AREA_Q:  result = m_client->ABWrite(start, data.size(), (void *)data.constData()); break;
    case AREA_DB: result = m_client->DBWrite(dbNumber, start, data.size(), (void *)data.constData()); break;
    default: return;
    }
    if (result == 0)
        emit dataWritten(areaType, dbNumber, start);
    else {
        QString msg = QString::fromUtf8(CliErrorText(result).c_str());
        emit errorOccurred(result, msg);
    }
}

// ------------------ 轮询（新增）------------------
void PlcWorker::setMonitorItems(const QVector<MonitorItem> &items)
{
    m_items = items;
}

void PlcWorker::startPolling(int intervalMs)
{
    m_timer->start(intervalMs);  // 启动定时器，每 intervalMs 毫秒触发一次
}

void PlcWorker::stopPolling()
{
    m_timer->stop();
}

void PlcWorker::pollAllItems()
{
    // ---- 自动重连检测 ----
    if (!m_client->Connected() && !m_lastIp.isEmpty() && !m_reconnecting) {
        m_reconnecting = true;
        QByteArray ipBytes = m_lastIp.toUtf8();
        m_client->ConnectTo(ipBytes.constData(), m_lastRack, m_lastSlot);
        m_reconnecting = false;

        if (m_client->Connected()) {
            m_wasConnected = true;
            m_consecutiveErrors = 0;
            emit connected();
        }
    }

    if (!m_client->Connected()) {
        if (m_wasConnected) {
            m_wasConnected = false;
            emit disconnected();
        }
        return;
    }

    // ---- 读取 CPU 状态 ----
    checkCpuStatus();

    // ---- 逐项读取 ----
    bool anyFailed = false;
    for (int i = 0; i < m_items.size(); ++i) {
        const MonitorItem &item = m_items[i];
        int size = sizeForType(item.valType);

        QByteArray buffer(size, '\0');
        int result = 0;
        switch (item.area) {
        case AREA_M:  result = m_client->MBRead(item.start, size, buffer.data()); break;
        case AREA_I:  result = m_client->EBRead(item.start, size, buffer.data()); break;
        case AREA_Q:  result = m_client->ABRead(item.start, size, buffer.data()); break;
        case AREA_DB: result = m_client->DBRead(item.dbNum, item.start, size, buffer.data()); break;
        }

        if (result == 0) {
            emit pollingData(i, item.area, item.dbNum, item.start, buffer);
        } else {
            anyFailed = true;
            emit pollingData(i, item.area, item.dbNum, item.start, QByteArray());
        }
    }

    // ---- 读失败即断线检测 ----
    if (anyFailed) {
        m_consecutiveErrors++;
        if (m_consecutiveErrors >= 3) {
            m_client->Disconnect();  // 强制断开 TCP，才能触发重连
            m_consecutiveErrors = 0;
        }
    } else {
        m_consecutiveErrors = 0;
    }

    emit pollingTick();
}

void PlcWorker::checkCpuStatus()
{
    int status = m_client->PlcStatus();
    emit cpuStatusChanged(status);
}

int PlcWorker::sizeForType(int valType) const
{
    switch (valType) {
    case 0: case 1: case 2: return 1;   // BOOL / INT8 / UINT8
    case 3: case 4:          return 2;   // INT16 / UINT16
    case 5: case 6:          return 4;   // INT32 / FLOAT32
    default:                 return 2;
    }
}
