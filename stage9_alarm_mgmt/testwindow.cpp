#include "testwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QtMath>

TestWindow::TestWindow(QWidget *parent) : QWidget(parent)
{
    m_alarmMan = new AlarmManager(this);
    applyTheme();
    setupUi();
    setupAlarmConditions();

    // 信号连接
    connect(m_alarmMan, &AlarmManager::alarmTriggered,
            this, &TestWindow::onAlarmTriggered);
    connect(m_alarmMan, &AlarmManager::countsChanged,
            this, &TestWindow::onCountsChanged);
    connect(m_banner, &AlarmBanner::clicked, this, &TestWindow::acknowledgeAll);
    connect(m_ackBtn, &QPushButton::clicked, this, &TestWindow::acknowledgeAll);
    connect(m_listWidget, &AlarmListWidget::acknowledgeRequested,
            this, [this](int id) { m_alarmMan->acknowledgeId(id); });

    m_simTimer = new QTimer(this);
    m_simTimer->setInterval(100);
    connect(m_simTimer, &QTimer::timeout, this, &TestWindow::tick);
    m_simTimer->start();

    setWindowTitle("Stage9 — Alarm Management Test");
}

void TestWindow::applyTheme()
{
    setStyleSheet(R"(
        QWidget {
            background-color: #1a1d23;
            color: #c0c0c0;
            font-family: "Segoe UI", "Microsoft YaHei";
        }
        QLabel { color: #c0c0c0; }
        QPushButton {
            background: #2d5a8c; border: none; border-radius: 4px;
            padding: 6px 16px; color: white; font-weight: bold;
        }
        QPushButton:hover { background: #3a6ea8; }
    )");
}

void TestWindow::setupUi()
{
    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(12, 8, 12, 8);
    root->setSpacing(6);

    // 标题
    QLabel *title = new QLabel("Process HMI — 报警管理系统测试");
    title->setStyleSheet("font-size: 16px; font-weight: bold; color: #e0e0e0;");
    root->addWidget(title);

    // ---- 报警横幅 ----
    m_banner = new AlarmBanner;
    root->addWidget(m_banner);

    // ---- 6 个标签仪表 ----
    QHBoxLayout *tagRow = new QHBoxLayout;
    tagRow->setSpacing(8);

    auto makeDisplay = [&](const QString &name, const QString &unit) {
        auto *d = new TagDisplay;
        d->setTagName(name); d->setUnit(unit);
        d->setFixedSize(140, 90);
        tagRow->addWidget(d);
        return d;
    };
    m_dispTemp  = makeDisplay("温度 TEMP", "°C");
    m_dispPress = makeDisplay("压力 PRESS", "MPa");
    m_dispTank  = makeDisplay("液位 TANK", "%");
    m_dispPump  = makeDisplay("转速 PUMP", "%");
    m_dispFlow  = makeDisplay("流量 FLOW", "m³/h");
    m_dispValve = makeDisplay("阀门 VALVE", "%");
    tagRow->addStretch();
    root->addLayout(tagRow);

    // ---- 报警列表标题 + 确认按钮 ----
    QHBoxLayout *listHeader = new QHBoxLayout;
    QLabel *listTitle = new QLabel("▎报警列表");
    listTitle->setStyleSheet("font-size: 13px; font-weight: bold; color: #e0e0e0;");
    listHeader->addWidget(listTitle);
    listHeader->addStretch();
    m_ackBtn = new QPushButton("确认全部");
    listHeader->addWidget(m_ackBtn);
    root->addLayout(listHeader);

    // ---- 报警列表 ----
    m_listWidget = new AlarmListWidget;
    m_listWidget->setMinimumHeight(220);
    root->addWidget(m_listWidget, 1);

    // ---- 说明 ----
    QLabel *hint = new QLabel("模拟正弦波触发报警 | 点击报警行确认单条 | 红色闪烁=未确认 | 回差防抖 2%");
    hint->setStyleSheet("color: #686868; font-size: 11px;");
    hint->setAlignment(Qt::AlignCenter);
    root->addWidget(hint);

    resize(950, 620);
}

void TestWindow::setupAlarmConditions()
{
    // TEMP: HH>90, H>80, L<10
    m_alarmMan->registerCondition({"TEMP",  90, 80, 10,  NAN});
    // PRESS: H>2.8, HH>3.5
    m_alarmMan->registerCondition({"PRESS", 3.5, 2.8, NAN, NAN});
    // TANK_LEVEL: L<15, LL<5
    m_alarmMan->registerCondition({"TANK",  NAN, NAN, 15, 5});
    // PUMP_SPEED: H>90, HH>95
    m_alarmMan->registerCondition({"PUMP",  95, 90, NAN, NAN});
    // FLOW: L<5
    m_alarmMan->registerCondition({"FLOW",  NAN, NAN, 5, NAN});
    // VALVE: no alarm conditions
}

void TestWindow::tick()
{
    m_simPhase += 0.04;

    // 各标签模拟值
    double temp  = 50 + 42 * qSin(m_simPhase * 0.5);           // 8~92, 周期性触发 H/HH/L
    double press = 1.5 + 2.2 * qSin(m_simPhase * 0.6 + 0.3);  // -0.7~3.7, 触发 H/HH
    double tank  = 18 + 20 * qSin(m_simPhase * 0.4 - 0.8);     // -2~38, 触发 L/LL
    double pump  = 80 + 18 * qSin(m_simPhase * 0.7 + 0.5);     // 62~98, 循环触发 H/HH
    double flow  = 8 + 10 * qSin(m_simPhase * 0.55 + 0.2);     // -2~18, 循环触发 L
    double valve = 50 + 30 * qSin(m_simPhase * 0.3);            // 20~80, 无报警

    // 更新显示
    m_dispTemp->setValue(temp);
    m_dispPress->setValue(press);
    m_dispTank->setValue(tank);
    m_dispPump->setValue(pump);
    m_dispFlow->setValue(flow);
    m_dispValve->setValue(valve);

    // 驱动 AlarmManager
    m_alarmMan->evaluateValue("TEMP",  temp);
    m_alarmMan->evaluateValue("PRESS", press);
    m_alarmMan->evaluateValue("TANK",  tank);
    m_alarmMan->evaluateValue("PUMP",  pump);
    m_alarmMan->evaluateValue("FLOW",  flow);
    // VALVE 不参与报警

    // 更新报警高亮
    auto checkAlarm = [this](const QString &tag) {
        int cnt = 0;
        for (const auto &r : m_alarmMan->records())
            if (r.tagName == tag && r.active) cnt++;
        return cnt > 0;
    };
    m_dispTemp->setAlarmActive(checkAlarm("TEMP"));
    m_dispPress->setAlarmActive(checkAlarm("PRESS"));
    m_dispTank->setAlarmActive(checkAlarm("TANK"));
    m_dispPump->setAlarmActive(checkAlarm("PUMP"));
    m_dispFlow->setAlarmActive(checkAlarm("FLOW"));

    // 更新列表
    m_listWidget->setAlarmRecords(m_alarmMan->records());
}

void TestWindow::onAlarmTriggered(const AlarmRecord &rec)
{
    m_lastUnacked = rec;
    m_hasLastUnacked = true;
}

void TestWindow::onCountsChanged(int active, int unacked)
{
    const AlarmRecord *latest = m_hasLastUnacked ? &m_lastUnacked : nullptr;
    if (unacked == 0) m_hasLastUnacked = false;
    m_banner->updateState(active, unacked, latest);
    m_ackBtn->setEnabled(unacked > 0);
}

void TestWindow::acknowledgeAll()
{
    m_alarmMan->acknowledgeAll();
}
