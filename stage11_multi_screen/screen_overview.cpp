#include "screen_overview.h"
#include "sim_data_manager.h"
#include <QGridLayout>
#include <QPainter>
#include <QDateTime>

// ── ScreenOverview ───────────────────────────────────────────────

ScreenOverview::ScreenOverview(QWidget *parent) : HmiScreen(parent)
{
    auto *grid = new QGridLayout(this);
    grid->setContentsMargins(20, 16, 20, 16);
    grid->setSpacing(12);

    setupCards();

    QStringList tags = {"TEMP", "PRESS", "TANK", "FLOW", "PUMP", "VALVE"};
    for (int i = 0; i < tags.size(); ++i) {
        int row = i / 3, col = i % 3;
        grid->addWidget(m_cards[i], row, col);
    }
}

void ScreenOverview::setupCards()
{
    m_cards.append(new OverviewCard("TEMP (°C)",  "°C",    this));
    m_cards.append(new OverviewCard("PRESS (MPa)", "MPa",   this));
    m_cards.append(new OverviewCard("TANK (%)",    "%",     this));
    m_cards.append(new OverviewCard("FLOW (m³/h)", "m³/h",  this));
    m_cards.append(new OverviewCard("PUMP (%)",    "%",     this));
    m_cards.append(new OverviewCard("VALVE (%)",   "%",     this));
}

void ScreenOverview::onEnter()
{
    if (!simData()) return;

    // connect data source to each card
    QStringList tags = {"TEMP", "PRESS", "TANK", "FLOW", "PUMP", "VALVE"};
    for (int i = 0; i < tags.size(); ++i) {
        connect(simData(), &SimDataManager::valueChanged, this,
                [this, i](const QString &tag, double v) {
                    QStringList t = {"TEMP", "PRESS", "TANK", "FLOW", "PUMP", "VALVE"};
                    if (tag == t[i]) {
                        m_cards[i]->setValue(v);
                        // simple threshold coloring
                        bool alarm = false, warn = false;
                        if (tag == "TEMP")  { alarm = v > 85; warn = v > 70; }
                        if (tag == "PRESS") { alarm = v > 3.2; warn = v > 2.5; }
                        if (tag == "TANK")  { alarm = v < 8;  warn = v < 15; }
                        if (tag == "FLOW")  { alarm = v < 3;  warn = v < 8; }
                        if (tag == "PUMP")  { alarm = v > 95; warn = v > 88; }
                        if (tag == "VALVE") { alarm = v < 15; warn = v < 30; }
                        m_cards[i]->setAlarm(alarm);
                        m_cards[i]->setWarning(warn && !alarm);
                    }
                });
        // initial value
        m_cards[i]->setValue(simData()->value(tags[i]));
    }
}

// ── OverviewCard ─────────────────────────────────────────────────

OverviewCard::OverviewCard(const QString &tagName, const QString &unit,
                           QWidget *parent)
    : QWidget(parent), m_tagName(tagName), m_unit(unit)
{
    setMinimumSize(200, 140);
}

void OverviewCard::setValue(double v)     { m_value = v; update(); }
void OverviewCard::setWarning(bool on)    { m_warning = on; update(); }
void OverviewCard::setAlarm(bool on)      { m_alarm = on; update(); }

void OverviewCard::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int w = width(), h = height();

    // Card background
    QColor bgBase(30, 33, 40);
    if (m_alarm)
        bgBase = QColor(60, 20, 20);
    else if (m_warning)
        bgBase = QColor(55, 40, 15);

    QLinearGradient grad(0, 0, 0, h);
    grad.setColorAt(0.0, bgBase.lighter(110));
    grad.setColorAt(1.0, bgBase);
    p.setPen(QPen(QColor(60, 65, 75), 1));
    p.setBrush(grad);
    p.drawRoundedRect(1, 1, w - 2, h - 2, 8, 8);

    // Status bar at bottom
    QColor statusColor(60, 180, 80);  // normal green
    if (m_alarm)  statusColor = QColor(240, 50, 50);
    else if (m_warning) statusColor = QColor(255, 150, 30);
    p.setPen(Qt::NoPen);
    p.setBrush(statusColor);
    p.drawRoundedRect(4, h - 8, w - 8, 5, 2, 2);

    // Tag name
    p.setPen(QColor(150, 155, 165));
    p.setFont(QFont("Microsoft YaHei", 9));
    p.drawText(QRect(12, 8, w - 24, 20), Qt::AlignLeft, m_tagName);

    // Value
    p.setPen(QColor(230, 235, 245));
    p.setFont(QFont("Consolas", 26, QFont::Bold));
    p.drawText(QRect(12, 30, w - 24, 50), Qt::AlignCenter,
               QString::number(m_value, 'f', 1));

    // Unit
    p.setPen(QColor(130, 135, 145));
    p.setFont(QFont("Microsoft YaHei", 9));
    p.drawText(QRect(12, 80, w - 24, 20), Qt::AlignRight, m_unit);
}
