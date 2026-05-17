#ifndef WIDGET_RENDER_HELPERS_H
#define WIDGET_RENDER_HELPERS_H

#include <QPainter>
#include <QPainterPath>
#include <QRectF>

namespace hmi {

// 3D 金属边框 — 4 段线性渐变模拟凹凸金属面板
void drawMetallicFrame(QPainter &p, const QRectF &rect, qreal radius,
                       const QColor &baseColor, bool raised = true);

// 发光光晕 — 3 段径向渐变，intensity 0.0~1.0
void drawGlowEffect(QPainter &p, const QPointF &center, qreal radius,
                    const QColor &color, qreal intensity = 0.5);

// 玻璃反光 — 白色→透明对角渐变，覆盖在给定路径上
void drawGlassReflection(QPainter &p, const QPainterPath &clipPath,
                         Qt::Orientation orient = Qt::Vertical);

// LED 状态指示点
void drawLEDIndicator(QPainter &p, const QPointF &center, qreal radius,
                      bool on, const QColor &onColor = QColor(40, 200, 60));

// 刻度尺 — 支持主副刻度
void drawRulerScale(QPainter &p, const QRectF &region, Qt::Orientation orient,
                    int numMajorTicks, const QColor &color);

// 弧形段 — 带圆角端帽
void drawSegmentedArc(QPainter &p, const QPointF &center, qreal radius,
                      qreal startAngle, qreal spanAngle, qreal thickness,
                      const QColor &color, bool filled = false);

// 文字阴影辅助：先画偏移阴影再画前景
void drawTextWithShadow(QPainter &p, const QRectF &rect, int flags,
                        const QString &text, const QColor &foreground,
                        const QColor &shadow = QColor(0, 0, 0, 100));

// 螺栓装饰 — 在指定位置画一个带金属渐变的圆
void drawBolt(QPainter &p, const QPointF &center, qreal radius);

} // namespace hmi

#endif
