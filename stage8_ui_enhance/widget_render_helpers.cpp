#include "widget_render_helpers.h"

namespace hmi {

void drawMetallicFrame(QPainter &p, const QRectF &rect, qreal radius,
                       const QColor &baseColor, bool raised)
{
    p.save();
    p.setPen(Qt::NoPen);

    QLinearGradient grad(rect.topLeft(), rect.bottomRight());
    if (raised) {
        grad.setColorAt(0.0, baseColor.lighter(180));
        grad.setColorAt(0.3, baseColor.lighter(120));
        grad.setColorAt(0.5, baseColor);
        grad.setColorAt(0.8, baseColor.darker(120));
        grad.setColorAt(1.0, baseColor.darker(160));
    } else {
        grad.setColorAt(0.0, baseColor.darker(160));
        grad.setColorAt(0.3, baseColor.darker(120));
        grad.setColorAt(0.5, baseColor);
        grad.setColorAt(0.8, baseColor.lighter(120));
        grad.setColorAt(1.0, baseColor.lighter(180));
    }
    p.setBrush(grad);
    p.drawRoundedRect(rect, radius, radius);
    p.restore();
}

void drawGlowEffect(QPainter &p, const QPointF &center, qreal radius,
                    const QColor &color, qreal intensity)
{
    p.save();
    p.setPen(Qt::NoPen);

    qreal r3 = radius * 1.8;
    QRadialGradient grad(center, r3);
    QColor c1 = color; c1.setAlphaF(intensity * 0.6);
    QColor c2 = color; c2.setAlphaF(intensity * 0.2);
    QColor c3 = color; c3.setAlphaF(0);
    grad.setColorAt(0.0, c1);
    grad.setColorAt(0.5, c2);
    grad.setColorAt(1.0, c3);
    p.setBrush(grad);
    p.drawEllipse(center, r3, r3);
    p.restore();
}

void drawGlassReflection(QPainter &p, const QPainterPath &clipPath,
                         Qt::Orientation orient)
{
    p.save();
    p.setClipPath(clipPath);
    p.setPen(Qt::NoPen);

    QRectF bounds = clipPath.boundingRect();
    QLinearGradient grad(bounds.topLeft(), bounds.bottomRight());
    QColor w1(255, 255, 255, 70);
    QColor w2(255, 255, 255, 0);
    if (orient == Qt::Vertical) {
        grad.setColorAt(0.0, w1);
        grad.setColorAt(0.4, w2);
    } else {
        grad.setColorAt(0.0, w1);
        grad.setColorAt(0.3, w2);
        grad.setColorAt(0.7, w2);
        grad.setColorAt(1.0, QColor(255, 255, 255, 30));
    }
    p.setBrush(grad);
    p.drawRect(bounds);
    p.restore();
}

void drawLEDIndicator(QPainter &p, const QPointF &center, qreal radius,
                      bool on, const QColor &onColor)
{
    p.save();
    p.setPen(Qt::NoPen);

    if (on) {
        // 光晕
        QColor glow = onColor; glow.setAlphaF(0.25);
        QRadialGradient glowGrad(center, radius * 3);
        glowGrad.setColorAt(0.0, glow);
        glowGrad.setColorAt(1.0, QColor(onColor.red(), onColor.green(), onColor.blue(), 0));
        p.setBrush(glowGrad);
        p.drawEllipse(center, radius * 3, radius * 3);

        // LED 本体
        QRadialGradient ledGrad(center + QPointF(-radius * 0.3, -radius * 0.3), radius);
        ledGrad.setColorAt(0.0, onColor.lighter(180));
        ledGrad.setColorAt(0.6, onColor);
        ledGrad.setColorAt(1.0, onColor.darker(150));
        p.setBrush(ledGrad);
    } else {
        QRadialGradient offGrad(center, radius);
        offGrad.setColorAt(0.0, QColor(100, 105, 110));
        offGrad.setColorAt(1.0, QColor(60, 63, 68));
        p.setBrush(offGrad);
    }
    p.drawEllipse(center, radius, radius);
    p.restore();
}

void drawRulerScale(QPainter &p, const QRectF &region, Qt::Orientation orient,
                    int numMajorTicks, const QColor &color)
{
    p.save();
    p.setPen(QPen(color, 1));
    QFont f = p.font();
    f.setPixelSize(qMax(8, (int)(region.width() * 0.22)));
    p.setFont(f);

    bool vertical = (orient == Qt::Vertical);
    qreal len = vertical ? region.height() : region.width();
    qreal start = vertical ? region.top() : region.left();
    qreal pos = vertical ? region.left() : region.top();

    for (int i = 0; i <= numMajorTicks; ++i) {
        qreal frac = (qreal)i / numMajorTicks;
        qreal y = start + len * frac;
        qreal tickLen = region.width() * 0.35; // major tick

        if (vertical)
            p.drawLine(QPointF(pos, y), QPointF(pos + tickLen, y));
        else
            p.drawLine(QPointF(y, pos), QPointF(y, pos + tickLen));

        // 标签（大刻度）
        QString label = QString::number((int)(100 - frac * 100));
        if (vertical) {
            QRectF labelRect(pos + tickLen + 2, y - 8, region.width() * 0.55, 16);
            p.drawText(labelRect, Qt::AlignLeft | Qt::AlignVCenter, label);
        }

        // 副刻度（4 条）
        if (i < numMajorTicks) {
            for (int j = 1; j <= 4; ++j) {
                qreal minorFrac = frac + (qreal)j / numMajorTicks / 5.0;
                qreal my = start + len * minorFrac;
                qreal minorLen = tickLen * 0.5;
                if (vertical)
                    p.drawLine(QPointF(pos, my), QPointF(pos + minorLen, my));
                else
                    p.drawLine(QPointF(my, pos), QPointF(my, pos + minorLen));
            }
        }
    }
    p.restore();
}

void drawSegmentedArc(QPainter &p, const QPointF &center, qreal radius,
                      qreal startAngle, qreal spanAngle, qreal thickness,
                      const QColor &color, bool filled)
{
    p.save();
    p.setPen(Qt::NoPen);
    p.setBrush(color);

    if (filled) {
        // 实心扇形
        QPainterPath path;
        path.moveTo(center);
        path.arcTo(QRectF(center.x() - radius, center.y() - radius,
                           radius * 2, radius * 2),
                   startAngle, spanAngle);
        path.closeSubpath();
        p.drawPath(path);
    } else {
        // 弧形条带
        qreal outerR = radius + thickness / 2;
        qreal innerR = radius - thickness / 2;
        QPainterPath path;
        // 外弧
        path.arcMoveTo(QRectF(center.x() - outerR, center.y() - outerR,
                              outerR * 2, outerR * 2), startAngle);
        path.arcTo(QRectF(center.x() - outerR, center.y() - outerR,
                          outerR * 2, outerR * 2),
                   startAngle, spanAngle);
        // 内弧（反向）
        path.arcTo(QRectF(center.x() - innerR, center.y() - innerR,
                          innerR * 2, innerR * 2),
                   startAngle + spanAngle, -spanAngle);
        path.closeSubpath();

        // 圆角端帽用两个小圆
        p.drawPath(path);

        p.setBrush(color.lighter(130));
        qreal startRad = qDegreesToRadians(startAngle);
        qreal endRad = qDegreesToRadians(startAngle + spanAngle);
        p.drawEllipse(QPointF(center.x() + (radius) * cos(startRad),
                              center.y() - (radius) * sin(startRad)),
                      thickness / 2, thickness / 2);
        p.drawEllipse(QPointF(center.x() + (radius) * cos(endRad),
                              center.y() - (radius) * sin(endRad)),
                      thickness / 2, thickness / 2);
    }
    p.restore();
}

void drawTextWithShadow(QPainter &p, const QRectF &rect, int flags,
                        const QString &text, const QColor &foreground,
                        const QColor &shadow)
{
    p.save();
    // 阴影
    p.setPen(shadow);
    QRectF shadowRect = rect.translated(1, 1);
    p.drawText(shadowRect, flags, text);
    // 前景
    p.setPen(foreground);
    p.drawText(rect, flags, text);
    p.restore();
}

void drawBolt(QPainter &p, const QPointF &center, qreal radius)
{
    p.save();
    p.setPen(Qt::NoPen);
    QRadialGradient grad(center + QPointF(-radius * 0.3, -radius * 0.3), radius);
    grad.setColorAt(0.0, QColor(200, 205, 210));
    grad.setColorAt(0.5, QColor(140, 145, 150));
    grad.setColorAt(1.0, QColor(80, 85, 90));
    p.setBrush(grad);
    p.drawEllipse(center, radius, radius);

    // 十字槽
    p.setPen(QPen(QColor(60, 65, 70), 1));
    qreal s = radius * 0.6;
    p.drawLine(QPointF(center.x() - s, center.y()), QPointF(center.x() + s, center.y()));
    p.drawLine(QPointF(center.x(), center.y() - s), QPointF(center.x(), center.y() + s));
    p.restore();
}

} // namespace hmi
