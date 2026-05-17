#ifndef SVG_RENDER_HELPERS_H
#define SVG_RENDER_HELPERS_H

#include <QPainter>
#include <QPainterPath>
#include <QPointF>
#include <QRectF>
#include <QColor>

namespace svg_helpers {

void drawGlowEffect(QPainter &p, const QPointF &center, qreal radius,
                    const QColor &color, qreal intensity = 0.5);

void drawLEDIndicator(QPainter &p, const QPointF &center, qreal radius,
                      bool on, const QColor &onColor = QColor(40, 200, 60));

void drawSegmentedArc(QPainter &p, const QPointF &center, qreal radius,
                      qreal startAngle, qreal spanAngle, qreal thickness,
                      const QColor &color, bool filled = false);

void drawTextWithShadow(QPainter &p, const QRectF &rect, int flags,
                        const QString &text, const QColor &foreground,
                        const QColor &shadow = QColor(0, 0, 0, 100));

} // namespace svg_helpers
#endif
