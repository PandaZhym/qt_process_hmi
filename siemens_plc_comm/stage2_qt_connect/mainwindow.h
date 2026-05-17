#pragma once
#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QThread>
#include "plc_worker.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    // 发给 worker 线程的信号（跨线程，Qt 自动排队执行）
    void requestConnect(const QString &ip, int rack, int slot);
    void requestDisconnect();

private slots:
    void onConnectClicked();
    void onDisconnectClicked();
    void onConnected();
    void onDisconnected();
    void onError(int code, const QString &message);

private:
    void setupUi();

    QLineEdit   *m_ipEdit;
    QPushButton *m_connectBtn;
    QPushButton *m_disconnectBtn;
    QLabel      *m_statusLabel;

    QThread     *m_thread;
    PlcWorker   *m_worker;
};
