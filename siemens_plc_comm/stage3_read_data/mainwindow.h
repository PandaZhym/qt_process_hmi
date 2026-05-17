#pragma once
#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QTextEdit>
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

private slots:
    // 连接相关
    void onConnectClicked();
    void onDisconnectClicked();
    void onConnected();
    void onDisconnected();
    void onError(int code, const QString &message);

    // 读取相关
    void onReadClicked();
    void onAreaChanged(int index);
    void onDataRead(int areaType, int dbNumber, int start, const QByteArray &data);

private:
    void setupUi();
    void setupConnections();
    QString formatHex(const QByteArray &data) const;
    QString parseValue(const QByteArray &data, int valueType) const;

    // ---- 连接控件 ----
    QLineEdit   *m_ipEdit;
    QPushButton *m_connectBtn;
    QPushButton *m_disconnectBtn;
    QLabel      *m_statusLabel;

    // ---- 读取控件 ----
    QComboBox   *m_areaCombo;      // 区域选择
    QSpinBox    *m_dbSpin;         // DB 号（仅 DB 区有效）
    QLabel      *m_dbLabel;
    QSpinBox    *m_startSpin;      // 起始地址
    QSpinBox    *m_sizeSpin;       // 读取长度（字节数）
    QComboBox   *m_typeCombo;      // 解析类型
    QPushButton *m_readBtn;
    QLabel      *m_hexLabel;       // 原始数据（HEX）
    QLabel      *m_valueLabel;     // 解析后的值

    // ---- 线程 ----
    QThread     *m_thread;
    PlcWorker   *m_worker;
};
