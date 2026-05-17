#include "snap7_adapter.h"
#include <cstring>

Snap7Adapter::Snap7Adapter(QObject *parent) : QObject(parent)
{
    m_client = new TS7Client();
    m_timer  = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &Snap7Adapter::poll);
}

Snap7Adapter::~Snap7Adapter()
{
    m_timer->stop();
    if (m_client->Connected())
        m_client->Disconnect();
    delete m_client;
}

void Snap7Adapter::connectToPlc(const QString &ip, int rack, int slot)
{
    m_lastIp   = ip;
    m_lastRack = rack;
    m_lastSlot = slot;
    int ret = m_client->ConnectTo(ip.toUtf8().constData(), rack, slot);
    if (ret == 0)
        emit connected();
    else
        emit errorOccurred(ret, QString::fromUtf8(CliErrorText(ret).c_str()));
}

void Snap7Adapter::disconnectFromPlc()
{
    m_timer->stop();
    m_client->Disconnect();
    emit disconnected();
}

void Snap7Adapter::setMappings(const QVector<TagMapping> &mappings)
{
    m_mappings = mappings;
}

void Snap7Adapter::startPolling(int intervalMs)
{
    m_timer->start(intervalMs);
}

void Snap7Adapter::stopPolling()
{
    m_timer->stop();
}

// ==================== 核心：ReadMultiVars 批量轮询 ====================
void Snap7Adapter::poll()
{
    // 自动重连
    if (!m_client->Connected()) {
        if (m_lastIp.isEmpty()) return;
        int ret = m_client->ConnectTo(m_lastIp.toUtf8().constData(),
                                      m_lastRack, m_lastSlot);
        if (ret != 0) {
            emit disconnected();
            return;
        }
    }

    int n = m_mappings.size();
    if (n == 0) return;

    QVector<TS7DataItem> items(n);
    QVector<QByteArray>  bufs(n);

    for (int i = 0; i < n; ++i) {
        const TagMapping &m = m_mappings[i];
        bufs[i].resize(m.size);

        items[i].Area     = m.area;
        items[i].WordLen  = S7WLByte;
        items[i].DBNumber = m.dbNum;
        items[i].Start    = m.start;
        items[i].Amount   = m.size;
        items[i].pdata    = bufs[i].data();
    }

    int ret = m_client->ReadMultiVars(items.data(), n);
    if (ret != 0) return;

    // 打包结果
    QHash<QString, QVariant> results;
    for (int i = 0; i < n; ++i) {
        const TagMapping &m = m_mappings[i];
        if (items[i].Result == 0)
            results[m.tagName] = parseValue(bufs[i], m.valType);
        else
            results[m.tagName] = QVariant();  // 无效值
    }

    if (!results.isEmpty())
        emit tagValuesReady(results);

    // CPU 状态
    emit cpuStatusChanged(m_client->PlcStatus());
}

// ==================== 大端字节序解析 ====================
QVariant Snap7Adapter::parseValue(const QByteArray &buf, int valType)
{
    switch (valType) {
    case 0:  // BOOL — 1 字节，非零为 true
        return (quint8)buf[0] != 0;
    case 1:  // INT8
        return (qint8)buf[0];
    case 2:  // UINT8
        return (quint8)buf[0];
    case 3:  // INT16 (大端)
        return qint16(((quint8)buf[0] << 8) | (quint8)buf[1]);
    case 4:  // UINT16 (大端)
        return quint16(((quint8)buf[0] << 8) | (quint8)buf[1]);
    case 5: { // INT32 (大端)
        quint32 bits = ((quint8)buf[0] << 24) | ((quint8)buf[1] << 16)
                     | ((quint8)buf[2] << 8)  | (quint8)buf[3];
        qint32 v;
        std::memcpy(&v, &bits, 4);
        return v;
    }
    case 6: { // FLOAT32 (大端)
        quint32 bits = ((quint8)buf[0] << 24) | ((quint8)buf[1] << 16)
                     | ((quint8)buf[2] << 8)  | (quint8)buf[3];
        float f;
        std::memcpy(&f, &bits, 4);
        return f;
    }
    default:
        return QVariant();
    }
}
