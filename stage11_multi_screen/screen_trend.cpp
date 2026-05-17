#include "screen_trend.h"
#include "sim_data_manager.h"
#include <QPainter>
#include <QPainterPath>
#include <QDateTime>
#include <QtMath>
#include <QLabel>
#include <QVBoxLayout>

ScreenTrend::ScreenTrend(QWidget *parent) : HmiScreen(parent)
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 8, 12, 8);

    auto *title = new QLabel("趋势曲线 / Trend Chart", this);
    title->setStyleSheet("color: #aaa; font-size: 11px; font-weight: bold;");
    mainLayout->addWidget(title);

    // chart area is self-painted (rest of widget)
    mainLayout->addStretch(1);

    // init curves
    m_curves.append({"TEMP",  QColor(240, 80, 60)});
    m_curves.append({"PRESS", QColor(60, 160, 240)});
    m_curves.append({"TANK",  QColor(100, 200, 100)});
}

void ScreenTrend::onEnter()
{
    // clear buffers on enter to start fresh
    for (auto &c : m_curves)
        c.samples.clear();
}

void ScreenTrend::onTick()
{
    if (!simData()) return;

    qint64 now = QDateTime::currentDateTime().toMSecsSinceEpoch();

    for (auto &c : m_curves) {
        double v = simData()->value(c.name);
        c.samples.append({now, v});
        while (c.samples.size() > MAX_SAMPLES)
            c.samples.removeFirst();
    }

    update();
}

void ScreenTrend::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int w = width(), h = height();
    QRect pr(MARGIN_L, MARGIN_T, w - MARGIN_L - MARGIN_R,
             h - MARGIN_T - MARGIN_B);

    if (pr.width() <= 0 || pr.height() <= 0) return;

    // Background
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(26, 29, 35));
    p.drawRoundedRect(pr.marginsAdded(QMargins(4, 4, 4, 4)), 6, 6);

    // Determine time range
    qint64 tMax = 0, tMin = 0;
    for (const auto &c : m_curves) {
        if (c.samples.isEmpty()) continue;
        if (tMax == 0) { tMax = c.samples.last().ts; tMin = tMax - 30000; }
    }
    if (tMax == 0) {
        // no data yet
        tMax = QDateTime::currentDateTime().toMSecsSinceEpoch();
        tMin = tMax - 30000;
    }
    qint64 tRange = tMax - tMin;

    // Determine Y range
    double yMin = 1e18, yMax = -1e18;
    for (const auto &c : m_curves) {
        for (const auto &s : c.samples) {
            if (s.ts < tMin || s.ts > tMax) continue;
            if (s.val < yMin) yMin = s.val;
            if (s.val > yMax) yMax = s.val;
        }
    }
    if (yMin > yMax) { yMin = 0; yMax = 100; }
    double padding = (yMax - yMin) * 0.1;
    if (padding < 1) padding = 1;
    double range = yMax - yMin + 2 * padding;
    double vBase = yMin - padding;

    // Grid
    p.setPen(QPen(QColor(40, 44, 50), 1, Qt::DotLine));
    for (int i = 0; i <= 5; ++i) {
        int gy = pr.bottom() - (int)(i * pr.height() / 5.0);
        p.drawLine(pr.left(), gy, pr.right(), gy);
    }
    for (int i = 0; i <= 6; ++i) {
        int gx = pr.left() + (int)(i * pr.width() / 6.0);
        p.drawLine(gx, pr.top(), gx, pr.bottom());
    }

    // Y-axis labels
    QFont labelFont("Consolas", 8);
    p.setFont(labelFont);
    p.setPen(QColor(130, 135, 145));
    for (int i = 0; i <= 5; ++i) {
        double v = vBase + range * i / 5.0;
        int gy = pr.bottom() - (int)(i * pr.height() / 5.0);
        QString s = QString::number(v, 'f', (range > 10) ? 0 : 1);
        p.drawText(QRect(0, gy - 8, MARGIN_L - 6, 16),
                   Qt::AlignRight | Qt::AlignVCenter, s);
    }

    // X-axis labels (seconds ago)
    for (int i = 0; i <= 6; ++i) {
        int gx = pr.left() + (int)(i * pr.width() / 6.0);
        int secsAgo = (int)((tMax - (tMin + tRange * i / 6.0)) / 1000);
        QString s = QString("-%1s").arg(secsAgo);
        p.drawText(QRect(gx - 25, pr.bottom() + 4, 50, 16),
                   Qt::AlignHCenter | Qt::AlignTop, s);
    }

    // Plot border
    p.setPen(QPen(QColor(60, 65, 75), 1));
    p.setBrush(Qt::NoBrush);
    p.drawRect(pr);

    // Curves
    for (const auto &c : m_curves) {
        if (c.samples.isEmpty()) continue;

        QPainterPath path;
        bool first = true;
        for (const auto &s : c.samples) {
            if (s.ts < tMin || s.ts > tMax) continue;
            double fx = (double)(s.ts - tMin) / tRange;
            double fy = (s.val - vBase) / range;
            QPointF pt(pr.left() + fx * pr.width(),
                       pr.bottom() - fy * pr.height());
            if (first) { path.moveTo(pt); first = false; }
            else       { path.lineTo(pt); }
        }
        p.setPen(QPen(c.color, 1.5));
        p.drawPath(path);
    }

    // Legend
    int lx = pr.right() - 10;
    int ly = pr.top() + 6;
    for (int i = 0; i < m_curves.size(); ++i) {
        const auto &c = m_curves[i];
        int ix = lx - 100;
        int iy = ly + i * 18;
        p.setPen(QPen(c.color, 2));
        p.drawLine(ix, iy + 9, ix + 16, iy + 9);
        p.setPen(QColor(200, 205, 215));
        p.setFont(QFont("Microsoft YaHei", 9));
        p.drawText(QRect(ix + 20, iy, 80, 18),
                   Qt::AlignLeft | Qt::AlignVCenter, c.name);
    }
}
