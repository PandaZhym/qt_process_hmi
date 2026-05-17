#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>

#include "tank_widget.h"
#include "pump_widget.h"
#include "valve_widget.h"
#include "pipe_widget.h"
#include "value_display.h"
#include "plc_worker.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onConnectClicked();
    void onDisconnectClicked();
    void onProcessDataUpdated(qreal tankLevel, qreal pumpSpeed, bool pumpRun,
                              qreal valveOpen, qreal temp,
                              qreal pressure, qreal flow);
    void onCpuStatusChanged(int status);
    void onConnected();
    void onDisconnected();
    void onError(int code, const QString &msg);

private:
    void setupUi();
    void setupWorker();
    void applyDarkTheme();

    // 控件
    TankWidget   *m_tank   = nullptr;
    PumpWidget   *m_pump   = nullptr;
    ValveWidget  *m_valve  = nullptr;
    PipeWidget   *m_pipe1  = nullptr, *m_pipe2 = nullptr, *m_pipe3 = nullptr;
    ValueDisplay *m_tempDisp = nullptr, *m_pressDisp = nullptr, *m_flowDisp = nullptr;

    // UI 元素
    QLineEdit   *m_ipEdit  = nullptr;
    QSpinBox    *m_rackSpin = nullptr, *m_slotSpin = nullptr;
    QSpinBox    *m_dbSpin   = nullptr;
    QPushButton *m_connBtn  = nullptr, *m_disconnBtn = nullptr;
    QLabel      *m_statusLabel = nullptr, *m_cpuLabel = nullptr;

    // 线程
    QThread   *m_thread = nullptr;
    PlcWorker *m_worker = nullptr;
};

#endif
