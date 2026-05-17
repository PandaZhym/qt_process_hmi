#include "testwindow.h"
#include "trend_chart.h"
#include "data_logger.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QDateTime>
#include <QDebug>

TestWindow::TestWindow(QWidget *parent) : QWidget(parent)
{
    setupUi();
    setupSimulation();

    m_logger = new DataLogger("trend_data.db", this);
    connect(m_logger, &DataLogger::error, this, &TestWindow::onLoggerError);
    m_logger->start();

    resize(1100, 700);
    setWindowTitle("Stage10 — Trend Chart + SQLite");
}

TestWindow::~TestWindow()
{
    if (m_simTimer) m_simTimer->stop();
    if (m_logger)   m_logger->stop();
}

// ── UI ───────────────────────────────────────────────────────────

void TestWindow::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(8);

    // ── Trend chart ──────────────────────────────────────
    m_chart = new TrendChart(this);
    m_chart->addCurve("TEMP",  QColor(240, 80, 60));
    m_chart->addCurve("PRESS", QColor(60, 160, 240));
    m_chart->addCurve("TANK",  QColor(100, 200, 100));
    m_chart->addCurve("FLOW",  QColor(220, 180, 40));
    m_chart->addCurve("PUMP",  QColor(180, 120, 220));
    m_chart->addCurve("VALVE", QColor(80, 200, 200));
    mainLayout->addWidget(m_chart, 1);

    // ── Control panel ────────────────────────────────────
    auto *ctrlGroup = new QGroupBox("控制面板", this);
    ctrlGroup->setStyleSheet(
        "QGroupBox { color: #ccc; font-weight: bold; border: 1px solid #444; "
        "border-radius: 4px; margin-top: 8px; padding-top: 16px; "
        "background: #1e2128; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 12px; }");
    auto *ctrlLayout = new QHBoxLayout(ctrlGroup);
    ctrlLayout->setSpacing(16);

    // Value labels
    auto makeLabel = [&](const QString &title, QLabel *&lbl) {
        auto *box = new QVBoxLayout;
        auto *titleLbl = new QLabel(title);
        titleLbl->setStyleSheet("color: #999; font-size: 9px;");
        titleLbl->setAlignment(Qt::AlignCenter);
        lbl = new QLabel("--");
        lbl->setAlignment(Qt::AlignCenter);
        lbl->setMinimumWidth(80);
        lbl->setStyleSheet(
            "color: #eee; font: bold 16px 'Consolas'; "
            "background: #16181d; border: 1px solid #333; "
            "border-radius: 4px; padding: 4px 10px;");
        box->addWidget(titleLbl);
        box->addWidget(lbl);
        ctrlLayout->addLayout(box);
    };

    makeLabel("TEMP (°C)", m_lblTemp);
    makeLabel("PRESS (MPa)", m_lblPress);
    makeLabel("TANK (%)", m_lblTank);
    makeLabel("FLOW (m³/h)", m_lblFlow);
    makeLabel("PUMP (%)", m_lblPump);
    makeLabel("VALVE (%)", m_lblValve);

    ctrlLayout->addStretch();

    m_btnHistory = new QPushButton("查询历史");
    m_btnHistory->setStyleSheet(
        "QPushButton { color: #eee; background: #2a5a3a; border: 1px solid #3a7a4a; "
        "border-radius: 4px; padding: 6px 16px; font-weight: bold; }"
        "QPushButton:hover { background: #3a7a4a; }");
    ctrlLayout->addWidget(m_btnHistory);
    connect(m_btnHistory, &QPushButton::clicked, this, &TestWindow::onQueryHistory);

    m_btnRealtime = new QPushButton("切回实时");
    m_btnRealtime->setStyleSheet(
        "QPushButton { color: #eee; background: #2a4a6a; border: 1px solid #3a6a8a; "
        "border-radius: 4px; padding: 6px 16px; font-weight: bold; }"
        "QPushButton:hover { background: #3a6a8a; }");
    ctrlLayout->addWidget(m_btnRealtime);
    connect(m_btnRealtime, &QPushButton::clicked, this, &TestWindow::onRealtimeMode);

    mainLayout->addWidget(ctrlGroup);
}

// ── Simulation ───────────────────────────────────────────────────

void TestWindow::setupSimulation()
{
    m_elapsed.start();
    m_simTimer = new QTimer(this);
    m_simTimer->setInterval(100);
    connect(m_simTimer, &QTimer::timeout, this, &TestWindow::onSimTick);
    m_simTimer->start();
}

void TestWindow::onSimTick()
{
    double t = m_elapsed.elapsed() / 1000.0;

    // Multi-frequency sine waves to look interesting on chart
    m_tempVal  = 50 + 25 * qSin(t * 0.3) + 8 * qSin(t * 0.7 + 1.2);
    m_pressVal = 1.5 + 0.8 * qSin(t * 0.45 + 0.5) + 0.3 * qSin(t * 1.1);
    m_tankVal  = 25 + 12 * qSin(t * 0.2 + 2.0) + 5 * qSin(t * 0.55);
    m_flowVal  = 15 + 6 * qSin(t * 0.5 + 1.0) + 3 * qSin(t * 0.85 + 0.8);
    m_pumpVal  = 80 + 15 * qSin(t * 0.35 + 3.0) + 5 * qSin(t * 0.6);
    m_valveVal = 50 + 25 * qSin(t * 0.15 + 1.5) + 10 * qSin(t * 0.4 + 2.5);

    qint64 now = QDateTime::currentDateTime().toMSecsSinceEpoch();

    auto feed = [&](const QString &tag, double val) {
        m_chart->appendData(tag, now, val);
        m_logger->logValue(tag, val, now);
    };

    feed("TEMP",  m_tempVal);
    feed("PRESS", m_pressVal);
    feed("TANK",  m_tankVal);
    feed("FLOW",  m_flowVal);
    feed("PUMP",  m_pumpVal);
    feed("VALVE", m_valveVal);

    // Update labels
    m_lblTemp->setText (QString::number(m_tempVal,  'f', 1));
    m_lblPress->setText(QString::number(m_pressVal, 'f', 2));
    m_lblTank->setText (QString::number(m_tankVal,  'f', 1));
    m_lblFlow->setText (QString::number(m_flowVal,  'f', 1));
    m_lblPump->setText (QString::number(m_pumpVal,  'f', 1));
    m_lblValve->setText(QString::number(m_valveVal, 'f', 1));
}

// ── History / Realtime toggle ────────────────────────────────────

void TestWindow::onQueryHistory()
{
    qint64 to   = QDateTime::currentDateTime().toMSecsSinceEpoch();
    qint64 from = to - 60000; // last 1 minute

    m_chart->setRealtimeMode(false);
    m_chart->setHistoryRange(from, to);

    // connect query results (one-shot would be cleaner, but manual disconnect works)
    connect(m_logger, &DataLogger::queryResult, m_chart,
            [this](const QString &tag, const QVector<QPair<qint64, double>> &data) {
                m_chart->loadHistory(tag, data);
            });

    m_logger->queryHistory("TEMP",  from, to);
    m_logger->queryHistory("PRESS", from, to);
    m_logger->queryHistory("TANK",  from, to);
    m_logger->queryHistory("FLOW",  from, to);
    m_logger->queryHistory("PUMP",  from, to);
    m_logger->queryHistory("VALVE", from, to);
}

void TestWindow::onRealtimeMode()
{
    m_chart->setRealtimeMode(true);
    // disconnect history query signal to avoid stale connections
    disconnect(m_logger, &DataLogger::queryResult, m_chart, nullptr);
}

void TestWindow::onLoggerError(const QString &msg)
{
    qWarning() << "DataLogger error:" << msg;
}
