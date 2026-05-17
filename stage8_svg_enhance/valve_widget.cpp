#include "valve_widget.h"
#include "svg_assets.h"
#include "svg_renderer_pool.h"
#include "svg_render_helpers.h"
#include <QPainter>
#include <QPainterPath>
#include <QSvgRenderer>
#include <QtMath>

ValveWidget::ValveWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(60, 60);
    m_anim = new QPropertyAnimation(this, "opening", this);
    m_anim->setDuration(400);
    m_anim->setEasingCurve(QEasingCurve::OutCubic);

    auto &pool = SvgRendererPool::instance();
    if (!pool.renderer("valve"))
        pool.add("valve", QByteArray(svg_assets::VALVE_BODY));
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

    int w = width(), h = height();
    int cx = w / 2, cy = h / 2 - 2;
    int sz = qMin(w, h) - 16;
    // SVG viewBox 140x160, 保持比例
    qreal svgW = sz * 140.0 / 160.0;
    qreal svgH = sz;
    QRectF svgRect(cx - svgW / 2.0, cy - svgH / 2.0, svgW, svgH);

    // ---- SVG 静态阀体 ----
    QSvgRenderer *valveSvg = SvgRendererPool::instance().renderer("valve");
    if (valveSvg) valveSvg->render(&p, svgRect);

    // ---- 扇形开度指示（QPainter 覆盖在阀体中心上方） ----
    // 阀体在 SVG 中的中心位置约 (70, 100)，阀盖/手轮在上面
    // 动态弧形画在阀体中心（SVG viewBox 约 y=100 的位置）
    qreal arcCx = cx;
    qreal arcCy = cy + svgH * 0.12; // 阀体中心偏下
    qreal arcRadius = svgW * 0.18;

    qreal openAngle = m_opening / 100.0 * 90.0;

    QColor arcColor;
    if (m_opening < 10)      arcColor = QColor(220, 50, 50);
    else if (m_opening < 40) arcColor = QColor(255, 170, 30);
    else if (m_opening < 70) arcColor = QColor(255, 200, 50);
    else                     arcColor = QColor(40, 200, 60);

    // 背景弧
    svg_helpers::drawSegmentedArc(p, QPointF(arcCx, arcCy), arcRadius,
                                  90, -90, svgW * 0.08, QColor(35, 40, 48), true);
    if (openAngle > 1) {
        svg_helpers::drawSegmentedArc(p, QPointF(arcCx, arcCy), arcRadius,
                                      90, -openAngle, svgW * 0.08, arcColor, true);
    }

    // 开度百分比
    QFont pctFont = font();
    pctFont.setPixelSize(svgW / 6);
    pctFont.setBold(true);
    p.setFont(pctFont);
    svg_helpers::drawTextWithShadow(p,
        QRectF(cx - svgW / 2, cy - svgH * 0.15, svgW, svgH * 0.4),
        Qt::AlignCenter, QString("%1%").arg(m_opening, 0, 'f', 0),
        Qt::white, QColor(0, 0, 0, 100));

    // 标签
    if (!m_label.isEmpty()) {
        p.setFont(QFont(font().family(), 9));
        p.setPen(QColor(200, 200, 200));
        p.drawText(QRect(0, cy + svgH / 2 + 2, w, 14),
                   Qt::AlignHCenter | Qt::AlignTop, m_label);
    }
}
