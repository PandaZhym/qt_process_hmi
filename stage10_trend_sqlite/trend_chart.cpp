#include "trend_chart.h"
#include <QPainter>
#include <QPainterPath>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QDateTime>
#include <QtMath>
#include <algorithm>

TrendChart::TrendChart(QWidget *parent) : QWidget(parent)
{
    setMouseTracking(true);
    setMinimumSize(400, 200);

    m_timeMax = QDateTime::currentDateTime().toMSecsSinceEpoch();
    m_timeMin = m_timeMax - DEFAULT_RANGE_MS;

    // 背景色
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(22, 25, 30));
    setPalette(pal);
    setAutoFillBackground(true);
}

void TrendChart::addCurve(const QString &name, const QColor &color)
{
    m_curves.append({name, color});
    m_rtData.insert(name, {});
}

void TrendChart::clearCurves()
{
    m_curves.clear();
    m_rtData.clear();
    m_histData.clear();
    update();
}

// ── Real-time ────────────────────────────────────────────────────

void TrendChart::appendData(const QString &curveName, qint64 timestamp, double value)
{
    if (!m_realtimeMode) return;
    if (!m_rtData.contains(curveName)) return;

    auto &buf = m_rtData[curveName];
    buf.append({timestamp, value});
    if (buf.size() > MAX_REALTIME_PTS)
        buf.remove(0, buf.size() - MAX_REALTIME_PTS);

    // advance time window
    m_timeMax = timestamp;
    m_timeMin = m_timeMax - DEFAULT_RANGE_MS;

    trimRealtimeData(curveName);
    if (m_autoYRange) recalcAutoYRange();
    update();
}

void TrendChart::trimRealtimeData(const QString &name)
{
    auto &buf = m_rtData[name];
    qint64 cutoff = m_timeMax - MAX_RANGE_MS; // keep last 60 min
    while (!buf.isEmpty() && buf.first().ts < cutoff)
        buf.removeFirst();
}

// ── History ──────────────────────────────────────────────────────

void TrendChart::loadHistory(const QString &curveName,
                             const QVector<QPair<qint64, double>> &data)
{
    QVector<Sample> samples;
    samples.reserve(data.size());
    for (const auto &p : data)
        samples.append({p.first, p.second});
    m_histData[curveName] = samples;
    update();
}

void TrendChart::setHistoryRange(qint64 fromTime, qint64 toTime)
{
    m_timeMin = fromTime;
    m_timeMax = toTime;
    if (m_autoYRange) recalcAutoYRange();
    update();
}

// ── Mode switch ──────────────────────────────────────────────────

void TrendChart::setRealtimeMode(bool realtime)
{
    m_realtimeMode = realtime;
    if (realtime) {
        m_histData.clear();
        m_timeMax = QDateTime::currentDateTime().toMSecsSinceEpoch();
        m_timeMin = m_timeMax - DEFAULT_RANGE_MS;
    }
    update();
}

void TrendChart::resetView()
{
    if (m_realtimeMode) {
        m_timeMax = QDateTime::currentDateTime().toMSecsSinceEpoch();
        m_timeMin = m_timeMax - DEFAULT_RANGE_MS;
    }
    m_autoYRange = true;
    recalcAutoYRange();
    update();
}

// ── Paint ────────────────────────────────────────────────────────

QRect TrendChart::plotRect() const
{
    const int L = 65, R = 18, T = 12, B = 38;
    return QRect(L, T, width() - L - R, height() - T - B);
}

QPointF TrendChart::dataToPixel(qint64 ts, double val, const QRect &r) const
{
    double x = r.left() + (double)(ts - m_timeMin) / (m_timeMax - m_timeMin) * r.width();
    double y = r.bottom() - (val - m_yMin) / (m_yMax - m_yMin) * r.height();
    return {x, y};
}

qint64 TrendChart::pixelToTime(int x, const QRect &r) const
{
    double ratio = (double)(x - r.left()) / r.width();
    return m_timeMin + (qint64)(ratio * (m_timeMax - m_timeMin));
}

qint64 TrendChart::niceTimeStep(qint64 rangeMs) const
{
    double seconds = rangeMs / 1000.0;
    // 目标 5~8 个刻度
    double raw = seconds / 6.0;
    // 向上取到常见值的倍数
    for (double s : {1.0, 2.0, 5.0, 10.0, 15.0, 30.0, 60.0,
                     120.0, 300.0, 600.0, 900.0, 1800.0, 3600.0}) {
        if (s >= raw) return (qint64)(s * 1000);
    }
    return 3600000;
}

double TrendChart::niceValueStep(double range) const
{
    double raw = range / 6.0;
    double mag = qPow(10, qFloor(qLn(raw) / qLn(10.0)));
    double norm = raw / mag;
    if (norm <= 1.5) return 1.0 * mag;
    if (norm <= 3.5) return 2.5 * mag;
    if (norm <= 7.5) return 5.0 * mag;
    return 10.0 * mag;
}

QString TrendChart::formatTime(qint64 tsMs, qint64 stepMs)
{
    QDateTime dt = QDateTime::fromMSecsSinceEpoch(tsMs);
    if (stepMs >= 60000)
        return dt.toString("HH:mm");
    return dt.toString("HH:mm:ss");
}

void TrendChart::recalcAutoYRange()
{
    double minV =  1e18;
    double maxV = -1e18;

    auto scan = [&](const QVector<Sample> &buf) {
        for (const auto &s : buf) {
            if (s.ts < m_timeMin || s.ts > m_timeMax) continue;
            if (s.val < minV) minV = s.val;
            if (s.val > maxV) maxV = s.val;
        }
    };

    if (m_realtimeMode) {
        for (auto it = m_rtData.begin(); it != m_rtData.end(); ++it)
            scan(it.value());
    } else {
        for (auto it = m_histData.begin(); it != m_histData.end(); ++it)
            scan(it.value());
    }

    if (minV > maxV) { minV = 0; maxV = 100; }

    double padding = (maxV - minV) * 0.1;
    if (padding < 1.0) padding = 1.0;
    m_yMin = minV - padding;
    m_yMax = maxV + padding;

    // round to nice values
    double step = niceValueStep(m_yMax - m_yMin);
    m_yMin = qFloor(m_yMin / step) * step;
    m_yMax = qCeil(m_yMax / step) * step;
}

void TrendChart::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int w = width(), h = height();
    QRect pr = plotRect();

    // ── Grid ──────────────────────────────────────────────
    p.setPen(QPen(QColor(40, 44, 50), 1));
    qint64 tStep = niceTimeStep(m_timeMax - m_timeMin);
    // snap to step boundary
    qint64 t0 = (m_timeMin / tStep) * tStep;
    if (t0 < m_timeMin) t0 += tStep;
    for (qint64 t = t0; t <= m_timeMax; t += tStep) {
        int x = (int)dataToPixel(t, 0, pr).x();
        if (x < pr.left() || x > pr.right()) continue;
        p.drawLine(x, pr.top(), x, pr.bottom());
    }

    qint64 rangeMs = m_timeMax - m_timeMin;
    double vStep = niceValueStep(m_yMax - m_yMin);
    double v0 = qFloor(m_yMin / vStep) * vStep;
    if (v0 < m_yMin) v0 += vStep;
    for (double v = v0; v <= m_yMax + vStep * 0.001; v += vStep) {
        int y = (int)dataToPixel(0, v, pr).y();
        if (y < pr.top() || y > pr.bottom()) continue;
        p.drawLine(pr.left(), y, pr.right(), y);
    }

    // ── Axis labels ───────────────────────────────────────
    QFont labelFont("Consolas", 9);
    p.setFont(labelFont);
    p.setPen(QColor(130, 135, 145));

    // X axis labels
    for (qint64 t = t0; t <= m_timeMax; t += tStep) {
        int x = (int)dataToPixel(t, 0, pr).x();
        if (x < pr.left() || x > pr.right()) continue;
        QString s = formatTime(t, tStep);
        QRect r(x - 30, pr.bottom() + 2, 60, 18);
        p.drawText(r, Qt::AlignHCenter | Qt::AlignTop, s);
    }

    // Y axis labels
    for (double v = v0; v <= m_yMax + vStep * 0.001; v += vStep) {
        int y = (int)dataToPixel(0, v, pr).y();
        if (y < pr.top() || y > pr.bottom()) continue;
        QString s = QString::number(v, 'f', (vStep < 1.0) ? 1 : 0);
        QRect r(0, y - 9, pr.left() - 6, 18);
        p.drawText(r, Qt::AlignRight | Qt::AlignVCenter, s);
    }

    // ── Plot border ───────────────────────────────────────
    p.setPen(QPen(QColor(60, 65, 75), 1));
    p.setBrush(Qt::NoBrush);
    p.drawRect(pr);

    // ── Curves ────────────────────────────────────────────
    auto drawCurve = [&](const QString &name, const QColor &color,
                         const QVector<Sample> &data) {
        QPainterPath path;
        bool first = true;
        for (const auto &s : data) {
            if (s.ts < m_timeMin || s.ts > m_timeMax) continue;
            QPointF pt = dataToPixel(s.ts, s.val, pr);
            if (first) { path.moveTo(pt); first = false; }
            else       { path.lineTo(pt); }
        }
        QPen pen(color, 1.5);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        p.drawPath(path);
    };

    if (m_realtimeMode) {
        for (const auto &cd : m_curves) {
            auto it = m_rtData.find(cd.name);
            if (it != m_rtData.end() && !it->isEmpty())
                drawCurve(cd.name, cd.color, *it);
        }
    } else {
        for (const auto &cd : m_curves) {
            auto it = m_histData.find(cd.name);
            if (it != m_histData.end() && !it->isEmpty())
                drawCurve(cd.name, cd.color, *it);
        }
    }

    // ── Legend ─────────────────────────────────────────────
    int lx = pr.right() - 10;
    int ly = pr.top() + 4;
    const int itemW = 110;
    const int itemH = 18;

    for (int i = m_curves.size() - 1; i >= 0; --i) {
        const auto &cd = m_curves[i];
        int ix = lx - itemW;
        int iy = ly + i * itemH;
        QRect itemRect(ix, iy, itemW, itemH - 2);

        // hover highlight
        if (m_hoveredLegend == i) {
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(255, 255, 255, 20));
            p.drawRoundedRect(itemRect, 3, 3);
        }

        // color line
        int cy = iy + itemH / 2;
        p.setPen(QPen(cd.color, 2));
        p.drawLine(ix + 4, cy, ix + 22, cy);

        // name
        p.setPen(QColor(200, 205, 215));
        QFont fn("Microsoft YaHei", 9);
        p.setFont(fn);
        p.drawText(QRect(ix + 26, iy, itemW - 30, itemH - 2),
                   Qt::AlignLeft | Qt::AlignVCenter, cd.name);
    }
}

// ── Interaction ──────────────────────────────────────────────────

void TrendChart::wheelEvent(QWheelEvent *event)
{
    QRect pr = plotRect();
    if (!pr.contains(event->position().toPoint())) return;

    qint64 range = m_timeMax - m_timeMin;
    qint64 center = m_timeMin + range / 2;

    double factor = (event->angleDelta().y() > 0) ? (1.0 / ZOOM_FACTOR) : ZOOM_FACTOR;
    qint64 newRange = (qint64)(range * factor);
    newRange = qBound(MIN_RANGE_MS, newRange, MAX_RANGE_MS);

    m_timeMin = center - newRange / 2;
    m_timeMax = center + newRange / 2;

    if (m_autoYRange) recalcAutoYRange();
    emit timeRangeChanged(m_timeMin, m_timeMax);
    update();
}

void TrendChart::mousePressEvent(QMouseEvent *event)
{
    QRect pr = plotRect();

    // check legend click first
    int lx = pr.right() - 10;
    int ly = pr.top() + 4;
    const int itemW = 110;
    const int itemH = 18;
    QPoint pos = event->pos();

    for (int i = 0; i < m_curves.size(); ++i) {
        int ix = lx - itemW;
        int iy = ly + i * itemH;
        QRect itemRect(ix, iy, itemW, itemH - 2);
        if (itemRect.contains(pos)) {
            m_curves[i].visible = !m_curves[i].visible;
            update();
            return;
        }
    }

    if (event->button() == Qt::LeftButton && pr.contains(pos)) {
        m_dragging = true;
        m_dragStartPos = pos;
        m_dragTimeMin = m_timeMin;
        m_dragTimeMax = m_timeMax;
        setCursor(Qt::ClosedHandCursor);
    }
}

void TrendChart::mouseMoveEvent(QMouseEvent *event)
{
    QRect pr = plotRect();
    QPoint pos = event->pos();

    if (m_dragging) {
        int dx = pos.x() - m_dragStartPos.x();
        double ratio = (double)dx / pr.width();
        qint64 shift = (qint64)(ratio * (m_dragTimeMax - m_dragTimeMin));
        m_timeMin = m_dragTimeMin - shift;
        m_timeMax = m_dragTimeMax - shift;

        if (m_autoYRange) recalcAutoYRange();
        emit timeRangeChanged(m_timeMin, m_timeMax);
        update();
        return;
    }

    // legend hover detection
    int lx = pr.right() - 10;
    int ly = pr.top() + 4;
    const int itemW = 110;
    const int itemH = 18;
    int hovered = -1;
    for (int i = 0; i < m_curves.size(); ++i) {
        int ix = lx - itemW;
        int iy = ly + i * itemH;
        if (QRect(ix, iy, itemW, itemH - 2).contains(pos)) {
            hovered = i;
            break;
        }
    }
    if (hovered != m_hoveredLegend) {
        m_hoveredLegend = hovered;
        update();
    }
}

void TrendChart::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_dragging) {
        m_dragging = false;
        setCursor(Qt::ArrowCursor);
    }
}

void TrendChart::mouseDoubleClickEvent(QMouseEvent *event)
{
    QRect pr = plotRect();
    if (event->button() == Qt::LeftButton && pr.contains(event->pos()))
        resetView();
}

void TrendChart::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    update();
}
