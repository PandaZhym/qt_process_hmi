#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>

#include "tag_manager.h"
#include "snap7_adapter.h"
#include "tank_widget.h"
#include "pump_widget.h"
#include "valve_widget.h"
#include "pipe_widget.h"
#include "value_display.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onConnectClicked();
    void onDisconnectClicked();
    void onConnected();
    void onDisconnected();
    void onError(int code, const QString &msg);
    void onTagValuesReady(const QHash<QString, QVariant> &values);
    void onTagChanged(const QString &name, const QVariant &value, bool valid);

private:
    void setupUi();
    void setupArchitecture();     // 组装 TagManager + Snap7Adapter
    void bindWidgets();           // 控件绑定标签
    void applyTheme();

    // 三层架构
    TagManager   *m_tagMan = nullptr;
    Snap7Adapter *m_adapter = nullptr;
    QThread      *m_thread = nullptr;

    // 画面控件
    TankWidget   *m_tank   = nullptr;
    PumpWidget   *m_pump   = nullptr;
    ValveWidget  *m_valve  = nullptr;
    PipeWidget   *m_pipe1  = nullptr, *m_pipe2 = nullptr, *m_pipe3 = nullptr;
    ValueDisplay *m_tempDisp = nullptr, *m_pressDisp = nullptr, *m_flowDisp = nullptr;

    // 连接 UI
    QLineEdit   *m_ipEdit    = nullptr;
    QSpinBox    *m_rackSpin  = nullptr, *m_slotSpin = nullptr;
    QPushButton *m_connBtn   = nullptr, *m_disconnBtn = nullptr;
    QLabel      *m_statusLbl = nullptr, *m_cpuLbl = nullptr;
};

#endif
