#include "screen_settings.h"
#include "sim_data_manager.h"
#include <QVBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QDateTime>

ScreenSettings::ScreenSettings(QWidget *parent) : HmiScreen(parent)
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 16, 20, 16);

    // ── Tag value table ──────────────────────────────────
    auto *tableGroup = new QGroupBox("模拟数据 / Simulated Tags", this);
    auto *tableLayout = new QVBoxLayout(tableGroup);

    m_table = new QTableWidget(6, 3, this);
    m_table->setHorizontalHeaderLabels({"标签名", "当前值", "单位"});
    m_table->verticalHeader()->setVisible(false);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setEditTriggers(QTableWidget::NoEditTriggers);
    m_table->setSelectionMode(QTableWidget::NoSelection);
    m_table->setColumnWidth(0, 120);
    m_table->setColumnWidth(1, 150);

    QStringList tags = {"TEMP", "PRESS", "TANK", "FLOW", "PUMP", "VALVE"};
    QStringList units = {"°C", "MPa", "%", "m³/h", "%", "%"};
    for (int i = 0; i < tags.size(); ++i) {
        m_table->setItem(i, 0, new QTableWidgetItem(tags[i]));
        m_table->setItem(i, 1, new QTableWidgetItem("--"));
        m_table->setItem(i, 2, new QTableWidgetItem(units[i]));
    }

    tableLayout->addWidget(m_table);
    mainLayout->addWidget(tableGroup);

    // ── System info ──────────────────────────────────────
    auto *infoGroup = new QGroupBox("系统信息 / System Info", this);
    auto *infoLayout = new QVBoxLayout(infoGroup);

    m_infoLabel = new QLabel(this);
    m_infoLabel->setStyleSheet("color: #bbb; font: 10px 'Consolas'; "
                               "padding: 6px;");
    m_infoLabel->setText(QString(
        "Stage11 — Multi-Screen Navigation Demo\n"
        "Qt %1 | C++17 | QPainter\n"
        "Global Tick: 100ms | Screens: 5\n"
        "Dark Theme | No PLC | Pure Simulation\n")
        .arg(qVersion()));
    infoLayout->addWidget(m_infoLabel);
    mainLayout->addWidget(infoGroup);

    mainLayout->addStretch(1);
}

void ScreenSettings::onEnter()
{
    onTick();
}

void ScreenSettings::onTick()
{
    if (!simData()) return;

    QStringList tags = {"TEMP", "PRESS", "TANK", "FLOW", "PUMP", "VALVE"};
    for (int i = 0; i < tags.size(); ++i) {
        double v = simData()->value(tags[i]);
        auto *item = m_table->item(i, 1);
        if (item) item->setText(QString::number(v, 'f', 2));
    }
}
