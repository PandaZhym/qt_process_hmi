#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();

    // ---- 创建 worker 并移入子线程 ----
    m_thread = new QThread(this);
    m_worker = new PlcWorker();        // worker 没有 parent，因为要 moveToThread
    m_worker->moveToThread(m_thread);

    // ---- 连接信号/槽 ----
    // UI → Worker（跨线程，Qt 自动用 QueuedConnection）
    connect(this, &MainWindow::requestConnect,
            m_worker, &PlcWorker::connectToPlc);
    connect(this, &MainWindow::requestDisconnect,
            m_worker, &PlcWorker::disconnectFromPlc);

    // Worker → UI（跨线程，更新界面控件）
    connect(m_worker, &PlcWorker::connected,
            this, &MainWindow::onConnected);
    connect(m_worker, &PlcWorker::disconnected,
            this, &MainWindow::onDisconnected);
    connect(m_worker, &PlcWorker::errorOccurred,
            this, &MainWindow::onError);

    // 线程退出时安全清理 worker
    connect(m_thread, &QThread::finished,
            m_worker, &QObject::deleteLater);

    m_thread->start();
}

MainWindow::~MainWindow()
{
    m_thread->quit();
    m_thread->wait();
}

// ---------- UI 搭建 ----------
void MainWindow::setupUi()
{
    setWindowTitle("S7-1200 连接测试");
    resize(400, 180);

    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout *vbox = new QVBoxLayout(central);

    // 第一行：IP 输入 + 连接按钮
    QHBoxLayout *hbox = new QHBoxLayout();
    m_ipEdit = new QLineEdit("192.168.0.1");
    m_connectBtn = new QPushButton("连接 PLC");
    hbox->addWidget(m_ipEdit);
    hbox->addWidget(m_connectBtn);
    vbox->addLayout(hbox);

    // 状态标签
    m_statusLabel = new QLabel("未连接");
    QFont font = m_statusLabel->font();
    font.setPointSize(18);
    m_statusLabel->setFont(font);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    vbox->addWidget(m_statusLabel);

    // 断开按钮
    m_disconnectBtn = new QPushButton("断开");
    m_disconnectBtn->setEnabled(false);
    vbox->addWidget(m_disconnectBtn);

    // 点击按钮 → 触发槽函数
    connect(m_connectBtn, &QPushButton::clicked,
            this, &MainWindow::onConnectClicked);
    connect(m_disconnectBtn, &QPushButton::clicked,
            this, &MainWindow::onDisconnectClicked);
}

// ---------- 槽：按钮点击 ----------
void MainWindow::onConnectClicked()
{
    m_statusLabel->setText("正在连接...");
    m_connectBtn->setEnabled(false);

    // 发信号给 worker 线程，真正连接在那边执行
    emit requestConnect(m_ipEdit->text(), 0, 1);
}

void MainWindow::onDisconnectClicked()
{
    emit requestDisconnect();
}

// ---------- 槽：Worker 返回的结果 ----------
void MainWindow::onConnected()
{
    m_statusLabel->setStyleSheet("color: green;");
    m_statusLabel->setText("已连接");
    m_connectBtn->setEnabled(false);
    m_disconnectBtn->setEnabled(true);
}

void MainWindow::onDisconnected()
{
    m_statusLabel->setStyleSheet("color: gray;");
    m_statusLabel->setText("未连接");
    m_connectBtn->setEnabled(true);
    m_disconnectBtn->setEnabled(false);
}

void MainWindow::onError(int code, const QString &message)
{
    m_statusLabel->setStyleSheet("color: red;");
    m_statusLabel->setText(QString("错误 %1: %2").arg(code).arg(message));
    m_connectBtn->setEnabled(true);
    m_disconnectBtn->setEnabled(false);
}
