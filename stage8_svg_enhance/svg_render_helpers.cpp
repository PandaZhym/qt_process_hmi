#include "svg_render_helpers.h"
#include <QtMath>

namespace svg_helpers {

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

void drawLEDIndicator(QPainter &p, const QPointF &center, qreal radius,
                      bool on, const QColor &onColor)
{
    p.save();
    p.setPen(Qt::NoPen);
    if (on) {
        QColor glow = onColor; glow.setAlphaF(0.25);
        QRadialGradient glowGrad(center, radius * 3);
        glowGrad.setColorAt(0.0, glow);
        glowGrad.setColorAt(1.0, QColor(onColor.red(), onColor.green(), onColor.blue(), 0));
        p.setBrush(glowGrad);
        p.drawEllipse(center, radius * 3, radius * 3);

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

void drawSegmentedArc(QPainter &p, const QPointF &center, qreal radius,
                      qreal startAngle, qreal spanAngle, qreal thickness,
                      const QColor &color, bool filled)
{
    p.save();
    p.setPen(Qt::NoPen);
    p.setBrush(color);
    if (filled) {
        QPainterPath path;
        path.moveTo(center);
        path.arcTo(QRectF(center.x() - radius, center.y() - radius,
                           radius * 2, radius * 2), startAngle, spanAngle);
        path.closeSubpath();
        p.drawPath(path);
    } else {
        qreal outerR = radius + thickness / 2;
        qreal innerR = radius - thickness / 2;
        QPainterPath path;
        path.arcMoveTo(QRectF(center.x() - outerR, center.y() - outerR,
                              outerR * 2, outerR * 2), startAngle);
        path.arcTo(QRectF(center.x() - outerR, center.y() - outerR,
                          outerR * 2, outerR * 2), startAngle, spanAngle);
        path.arcTo(QRectF(center.x() - innerR, center.y() - innerR,
                          innerR * 2, innerR * 2), startAngle + spanAngle, -spanAngle);
        path.closeSubpath();
        p.drawPath(path);

        p.setBrush(color.lighter(130));
        qreal startRad = qDegreesToRadians(startAngle);
        qreal endRad = qDegreesToRadians(startAngle + spanAngle);
        p.drawEllipse(QPointF(center.x() + radius * cos(startRad),
                              center.y() - radius * sin(startRad)), thickness / 2, thickness / 2);
        p.drawEllipse(QPointF(center.x() + radius * cos(endRad),
                              center.y() - radius * sin(endRad)), thickness / 2, thickness / 2);
    }
    p.restore();
}

void drawTextWithShadow(QPainter &p, const QRectF &rect, int flags,
                        const QString &text, const QColor &foreground,
                        const QColor &shadow)
{
    p.save();
    p.setPen(shadow);
    QRectF shadowRect = rect.translated(1, 1);
    p.drawText(shadowRect, flags, text);
    p.setPen(foreground);
    p.drawText(rect, flags, text);
    p.restore();
}

} // namespace svg_helpers
