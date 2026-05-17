#include "pump_widget.h"
#include "svg_assets.h"
#include "svg_renderer_pool.h"
#include "svg_render_helpers.h"
#include <QPainter>
#include <QPainterPath>
#include <QSvgRenderer>
#include <QtMath>

PumpWidget::PumpWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(80, 80);
    m_animTimer = new QTimer(this);
    m_animTimer->setInterval(30);
    connect(m_animTimer, &QTimer::timeout, this, [this]() {
        m_rotation += m_speed * 0.1;
        if (m_rotation >= 360) m_rotation -= 360;
        m_glowPhase += 0.05;
        if (m_glowPhase > 2 * M_PI) m_glowPhase -= 2 * M_PI;
        update();
    });

    auto &pool = SvgRendererPool::instance();
    if (!pool.renderer("pump"))
        pool.add("pump", QByteArray(svg_assets::PUMP_BODY));
}

void PumpWidget::setRunning(bool on)
{
    if (m_running == on) return;
    m_running = on;
    if (on) m_animTimer->start();
    else { m_animTimer->stop(); m_speed = 0; m_rotation = 0; m_glowPhase = 0; }
    update();
}

void PumpWidget::setSpeed(qreal pct)
{
    m_speed = qBound(0.0, pct, 100.0);
    if (!m_running && m_speed > 0) setRunning(true);
    else if (m_speed == 0 && m_running) setRunning(false);
}

void PumpWidget::setRotation(qreal deg) { m_rotation = deg; update(); }

void PumpWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int w = width(), h = height();
    int cx = w / 2, cy = h / 2 - 6;
    int svgSz = qMin(w, h) - 16;
    QRectF svgRect(cx - svgSz / 2.0, cy - svgSz / 2.0, svgSz, svgSz * 160.0 / 180.0);

    // 运转光晕
    if (m_running) {
        qreal glowIntensity = 0.15 + 0.1 * qSin(m_glowPhase);
        svg_helpers::drawGlowEffect(p, QPointF(cx, cy), svgSz * 0.5,
                                    QColor(30, 200, 80), glowIntensity);
    }

    // ---- SVG 静态泵体 ----
    QSvgRenderer *pumpSvg = SvgRendererPool::instance().renderer("pump");
    if (pumpSvg) pumpSvg->render(&p, svgRect);

    // ---- QPainter 动态叶轮（中心区域） ----
    int r = svgSz * 0.18;
    p.save();
    p.translate(cx, cy);
    p.rotate(m_rotation);

    QColor bladeColor = m_running ? QColor(220, 230, 240) : QColor(140, 145, 150);
    p.setBrush(bladeColor);
    p.setPen(Qt::NoPen);
    for (int i = 0; i < 3; ++i) {
        p.rotate(120);
        QPainterPath blade;
        qreal innerR = r * 0.2, outerR = r * 0.75;
        blade.moveTo(innerR * cos(-0.15), innerR * sin(-0.15));
        blade.cubicTo(innerR * cos(0.2) + outerR * 0.3, innerR * sin(0.2),
                      outerR * cos(0.5) + innerR * 0.3, outerR * sin(0.5),
                      outerR * cos(0.4), outerR * sin(0.4));
        blade.cubicTo(outerR * cos(0.3) + innerR * 0.3, outerR * sin(0.3) - innerR * 0.15,
                      innerR * cos(-0.05) + outerR * 0.2, innerR * sin(-0.05) - innerR * 0.2,
                      innerR * cos(-0.25), innerR * sin(-0.25));
        blade.closeSubpath();
        p.drawPath(blade);
    }
    p.restore();

    // 中心轴点
    QRadialGradient hubGrad(cx - r * 0.08, cy - r * 0.08, r * 0.25);
    hubGrad.setColorAt(0.0, QColor(200, 205, 210));
    hubGrad.setColorAt(0.5, QColor(150, 155, 160));
    hubGrad.setColorAt(1.0, QColor(90, 95, 100));
    p.setBrush(hubGrad);
    p.setPen(QPen(QColor(70, 75, 80), 1));
    p.drawEllipse(QPointF(cx, cy), r * 0.2, r * 0.2);

    // 转速弧形
    if (m_running && m_speed > 0) {
        qreal span = m_speed / 100.0 * 270.0;
        QColor arcColor;
        if (m_speed < 30)      arcColor = QColor(40, 200, 60);
        else if (m_speed < 60) arcColor = QColor(255, 200, 50);
        else if (m_speed < 85) arcColor = QColor(255, 150, 30);
        else                   arcColor = QColor(240, 50, 50);
        svg_helpers::drawSegmentedArc(p, QPointF(cx, cy),
                                      svgSz * 0.46, 45, -span, 4, arcColor);
    }

    // 标签
    if (!m_label.isEmpty()) {
        p.setPen(m_running ? QColor(60, 220, 80) : QColor(180, 180, 180));
        p.setFont(QFont(font().family(), 10));
        p.drawText(QRect(0, cy + svgSz * 0.5 + 4, w, 14),
                   Qt::AlignHCenter | Qt::AlignTop, m_label);
    }
}
