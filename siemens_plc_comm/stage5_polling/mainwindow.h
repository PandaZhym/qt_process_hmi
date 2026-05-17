#pragma once
#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QTableWidget>
#include <QThread>
#include "plc_worker.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void requestConnect(const QString &ip, int rack, int slot);
    void requestDisconnect();
    void requestRead(int areaType, int dbNumber, int start, int size);
    void requestWrite(int areaType, int dbNumber, int start,
                      const QByteArray &data);
    // 新增
    void requestStartPolling(int intervalMs);
    void requestStopPolling();
    void requestSetMonitorItems(const QVector<MonitorItem> &items);

private slots:
    void onConnectClicked();
    void onDisconnectClicked();
    void onConnected();
    void onDisconnected();
    void onError(int code, const QString &message);

    void onReadClicked();
    void onDataRead(int areaType, int dbNumber, int start,
                    const QByteArray &data);

    void onWriteClicked();
    void onDataWritten(int areaType, int dbNumber, int start);

    // 新增：轮询
    void onAddMonitorItem();
    void onRemoveMonitorItem();
    void onStartPolling();
    void onStopPolling();
    void onPollingData(int idx, int area, int dbNum, int start,
                       const QByteArray &data);
    void onCpuStatusChanged(int status);
    void syncItemsToWorker();

private:
    void setupUi();
    void setupConnections();
    QString formatHex(const QByteArray &data) const;
    QString parseValue(const QByteArray &data, int valueType) const;
    QByteArray packValue(int valueType, const QString &text) const;

    // ---- 连接 ----
    QLineEdit   *m_ipEdit;
    QPushButton *m_connectBtn;
    QPushButton *m_disconnectBtn;
    QLabel      *m_statusLabel;
    QLabel      *m_cpuLabel;

    // ---- 监控表 ----
    QTableWidget *m_table;
    QPushButton  *m_addBtn;
    QPushButton  *m_removeBtn;

    // ---- 轮询 ----
    QSpinBox    *m_intervalSpin;
    QPushButton *m_startPollBtn;
    QPushButton *m_stopPollBtn;

    // ---- 手动读写 ----
    QComboBox   *m_rwAreaCombo;
    QSpinBox    *m_rwDbSpin;
    QLabel      *m_rwDbLabel;
    QSpinBox    *m_rwStartSpin;
    QComboBox   *m_rwTypeCombo;
    QPushButton *m_readBtn;
    QLineEdit   *m_rwValueEdit;
    QPushButton *m_writeBtn;
    QLabel      *m_rwHexLabel;
    QLabel      *m_rwValueLabel;
    QLabel      *m_rwResultLabel;

    // ---- 线程 ----
    QThread     *m_thread;
    PlcWorker   *m_worker;
};
