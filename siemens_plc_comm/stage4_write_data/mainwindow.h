#pragma once
#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
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
    // 新增：写请求
    void requestWrite(int areaType, int dbNumber, int start,
                      const QByteArray &data);

private slots:
    // 连接
    void onConnectClicked();
    void onDisconnectClicked();
    void onConnected();
    void onDisconnected();
    void onError(int code, const QString &message);

    // 读取
    void onReadClicked();
    void onReadAreaChanged(int index);
    void onDataRead(int areaType, int dbNumber, int start,
                    const QByteArray &data);

    // 写入（新增）
    void onWriteClicked();
    void onWriteAreaChanged(int index);
    void onDataWritten(int areaType, int dbNumber, int start);

private:
    void setupUi();
    void setupConnections();

    // 工具函数
    QString formatHex(const QByteArray &data) const;
    QString parseValue(const QByteArray &data, int valueType) const;
    // 新增：把用户输入的值打包成大端字节序
    QByteArray packValue(int valueType, const QString &text) const;

    // ---- 连接控件 ----
    QLineEdit   *m_ipEdit;
    QPushButton *m_connectBtn;
    QPushButton *m_disconnectBtn;
    QLabel      *m_statusLabel;

    // ---- 读取控件 ----
    QComboBox   *m_readAreaCombo;
    QSpinBox    *m_readDbSpin;
    QLabel      *m_readDbLabel;
    QSpinBox    *m_readStartSpin;
    QSpinBox    *m_readSizeSpin;
    QComboBox   *m_readTypeCombo;
    QPushButton *m_readBtn;
    QLabel      *m_hexLabel;
    QLabel      *m_valueLabel;

    // ---- 写入控件（新增）----
    QComboBox   *m_writeAreaCombo;
    QSpinBox    *m_writeDbSpin;
    QLabel      *m_writeDbLabel;
    QSpinBox    *m_writeStartSpin;
    QComboBox   *m_writeTypeCombo;
    QLineEdit   *m_writeValueEdit;
    QPushButton *m_writeBtn;
    QLabel      *m_writeResultLabel;

    // ---- 线程 ----
    QThread     *m_thread;
    PlcWorker   *m_worker;
};
