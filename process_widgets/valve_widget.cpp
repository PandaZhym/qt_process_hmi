#include "valve_widget.h"
#include <QPainter>
#include <QPainterPath>
#include <QtMath>

ValveWidget::ValveWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(60, 60);
}

void ValveWidget::setOpening(qreal pct)
{
    pct = qBound(0.0, pct, 100.0);
    if (qFuzzyCompare(m_opening, pct)) return;
    m_opening = pct;
    update();
}

void ValveWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int w = width();
    int h = height();
    int cx = w / 2, cy = h / 2 - 2;
    int sz = qMin(w, h) - 16;

    QColor base = m_opening > 5 ? QColor(40, 160, 60) : QColor(180, 50, 50);

    // 阀体背景
    QPainterPath body;
    body.addRoundedRect(cx - sz/2, cy - sz/2, sz, sz, sz/5, sz/5);
    p.setBrush(QColor(60, 65, 70));
    p.setPen(QPen(QColor(140, 145, 150), 2));
    p.drawPath(body);

    // 开度三角指示
    qreal angle = m_opening / 100.0 * 90.0;  // 0°到 90°
    qreal rad = qDegreesToRadians(angle);
    int triSize = sz * 0.35;

    p.save();
    p.translate(cx, cy);
    p.rotate(-45);
    p.setBrush(base);
    p.setPen(Qt::NoPen);
    QPainterPath tri;
    tri.moveTo(-triSize * 0.2, 0);
    tri.lineTo(triSize * 0.8, -triSize * 0.6);
    tri.lineTo(triSize * 0.8, triSize * 0.6);
    tri.closeSubpath();
    p.drawPath(tri);
    p.restore();

    // 中心圆
    p.setBrush(QColor(40, 45, 50));
    p.setPen(QPen(QColor(160, 165, 170), 1));
    p.drawEllipse(QPointF(cx, cy), sz * 0.15, sz * 0.15);

    // 开度百分比
    p.setPen(Qt::white);
    p.setFont(QFont(font().family(), sz/5, QFont::Bold));
    p.drawText(QRect(0, cy - sz/4, w, sz/2), Qt::AlignCenter,
               QString("%1%").arg(m_opening, 0, 'f', 0));

    // 标签
    if (!m_label.isEmpty()) {
        p.setPen(QColor(200, 200, 200));
        p.setFont(QFont(font().family(), 9));
        p.drawText(QRect(0, cy + sz/2 + 2, w, 14), Qt::AlignHCenter | Qt::AlignTop, m_label);
    }
}
