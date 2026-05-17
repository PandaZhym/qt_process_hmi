#include "nav_bar.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDateTime>
#include <QtMath>

NavBar::NavBar(QWidget *parent) : QWidget(parent)
{
    setFixedHeight(BAR_HEIGHT);
    setMouseTracking(true);

    m_clockNow = QTime::currentTime();

    m_clockTimer = new QTimer(this);
    m_clockTimer->setInterval(1000);
    connect(m_clockTimer, &QTimer::timeout, this, &NavBar::refreshClock);
    m_clockTimer->start();
}

void NavBar::addButton(const ButtonDef &btn)
{
    m_buttons.append({btn, QRectF()});
    recalcLayout();
    update();
}

void NavBar::setActiveIndex(int index)
{
    if (index >= 0 && index < m_buttons.size() && index != m_activeIndex) {
        m_activeIndex = index;
        update();
    }
}

void NavBar::setAlarmCount(int active, int unacked)
{
    if (m_alarmActive != active || m_alarmUnacked != unacked) {
        m_alarmActive  = active;
        m_alarmUnacked = unacked;
        update();
    }
}

void NavBar::refreshClock()
{
    m_clockNow = QTime::currentTime();
    update();
}

QRectF NavBar::buttonRect(int index) const
{
    if (index < 0 || index >= m_buttons.size()) return {};
    int x = 10 + index * (BTN_WIDTH + 4);
    return QRectF(x, BTN_TOP, BTN_WIDTH, BTN_HEIGHT);
}

void NavBar::recalcLayout()
{
    for (int i = 0; i < m_buttons.size(); ++i)
        m_buttons[i].bounds = buttonRect(i);
}

void NavBar::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    recalcLayout();
}

// ── Paint ────────────────────────────────────────────────────────

void NavBar::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int w = width();
    int h = height();

    // Background
    QLinearGradient bg(0, 0, 0, h);
    bg.setColorAt(0.0, QColor(30, 33, 40));
    bg.setColorAt(1.0, QColor(22, 25, 30));
    p.fillRect(0, 0, w, h, bg);

    // Bottom separator
    p.setPen(QPen(QColor(50, 55, 65), 1));
    p.drawLine(0, h - 1, w, h - 1);

    // Buttons
    QFont iconFont("Segoe UI Symbol", 14);
    QFont labelFont("Microsoft YaHei", 10);

    for (int i = 0; i < m_buttons.size(); ++i) {
        const auto &btn = m_buttons[i];
        QRectF r = btn.bounds;

        // Button background based on state
        if (i == m_activeIndex) {
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(42, 90, 138));
            p.drawRoundedRect(r, 5, 5);

            // Top accent bar
            p.setBrush(QColor(74, 154, 255));
            p.drawRoundedRect(QRectF(r.x() + 4, r.y() + 1, r.width() - 8, 3), 1, 1);
        } else if (i == m_hoveredIndex) {
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(255, 255, 255, 12));
            p.drawRoundedRect(r, 5, 5);
        }

        // Icon
        p.setFont(iconFont);
        QColor textColor = (i == m_activeIndex) ? QColor(230, 240, 255) : QColor(150, 155, 165);
        p.setPen(textColor);
        p.drawText(QRectF(r.x() + 6, r.y(), 24, r.height()),
                   Qt::AlignCenter, btn.def.icon);

        // Label
        p.setFont(labelFont);
        p.drawText(QRectF(r.x() + 26, r.y(), r.width() - 30, r.height()),
                   Qt::AlignLeft | Qt::AlignVCenter, btn.def.label);
    }

    // ── Right: clock ─────────────────────────────────────
    int clockX = w - 230;
    QFont clockFont("Consolas", 11);
    p.setFont(clockFont);
    p.setPen(QColor(180, 185, 195));

    QString dateStr = QDate::currentDate().toString("yyyy-MM-dd");
    QString timeStr = m_clockNow.toString("HH:mm:ss");
    QString clockText = dateStr + "  " + timeStr;
    QRect clockRect(clockX, 0, 240, h);
    p.drawText(clockRect, Qt::AlignRight | Qt::AlignVCenter, clockText);

    // ── Alarm indicator ───────────────────────────────────
    if (m_alarmUnacked > 0) {
        int dotX = clockX - 24;
        int dotY = (h - 12) / 2;
        int dotR = 6;

        // Pulsing glow
        int alpha = 80 + 40 * qSin(QDateTime::currentMSecsSinceEpoch() / 400.0 * M_PI);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(240, 50, 50, alpha));
        p.drawEllipse(QPointF(dotX, dotY + 6), dotR + 4, dotR + 4);

        // Core dot
        p.setBrush(QColor(240, 50, 50));
        p.drawEllipse(QPointF(dotX, dotY + 6), dotR, dotR);

        // Count
        QFont countFont("Consolas", 8, QFont::Bold);
        p.setFont(countFont);
        p.setPen(Qt::white);
        p.drawText(QRectF(dotX - dotR - 2, dotY, 2 * dotR + 4, 12),
                   Qt::AlignCenter, QString::number(m_alarmUnacked));
    }
}

// ── Mouse ────────────────────────────────────────────────────────

void NavBar::mousePressEvent(QMouseEvent *event)
{
    QPoint pos = event->pos();
    for (int i = 0; i < m_buttons.size(); ++i) {
        if (m_buttons[i].bounds.contains(pos)) {
            emit screenSelected(i);
            return;
        }
    }
}

void NavBar::mouseMoveEvent(QMouseEvent *event)
{
    QPoint pos = event->pos();
    int prev = m_hoveredIndex;
    m_hoveredIndex = -1;
    for (int i = 0; i < m_buttons.size(); ++i) {
        if (m_buttons[i].bounds.contains(pos)) {
            m_hoveredIndex = i;
            break;
        }
    }
    if (m_hoveredIndex != prev)
        update();
}

void NavBar::leaveEvent(QEvent *)
{
    if (m_hoveredIndex >= 0) {
        m_hoveredIndex = -1;
        update();
    }
}
