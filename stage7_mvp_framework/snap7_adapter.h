#ifndef SNAP7_ADAPTER_H
#define SNAP7_ADAPTER_H

#include <QObject>
#include <QTimer>
#include <QHash>
#include <QVariant>
#include <QVector>
#include <QByteArray>
#include "snap7.h"

// 一条标签映射 — 把"标签名"关联到"PLC 地址"
struct TagMapping {
    QString tagName;
    int area    = S7AreaDB;
    int dbNum   = 1;
    int start   = 0;         // 起始字节
    int size    = 2;         // 读取的字节数
    int valType = 4;         // 解析类型: 0=BOOL, 4=UINT16, 6=FLOAT32 等
};

// 通讯层 — 封装 snap7 TS7Client
class Snap7Adapter : public QObject
{
    Q_OBJECT

public:
    explicit Snap7Adapter(QObject *parent = nullptr);
    ~Snap7Adapter() override;

    bool isConnected() const { return m_client->Connected(); }

public slots:
    void connectToPlc(const QString &ip, int rack, int slot);
    void disconnectFromPlc();
    void setMappings(const QVector<TagMapping> &mappings);
    void startPolling(int intervalMs = 500);
    void stopPolling();

signals:
    void connected();
    void disconnected();
    void errorOccurred(int code, const QString &msg);
    void tagValuesReady(const QHash<QString, QVariant> &values);
    void cpuStatusChanged(int status);

private slots:
    void poll();                  // ReadMultiVars 一次读全部

private:
    QVariant parseValue(const QByteArray &buf, int valType);

    TS7Client *m_client = nullptr;
    QTimer    *m_timer  = nullptr;
    QVector<TagMapping> m_mappings;
    QString m_lastIp;
    int m_lastRack = 0, m_lastSlot = 1;
};

#endif
