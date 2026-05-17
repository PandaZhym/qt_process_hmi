#include "pump_widget.h"
#include "widget_render_helpers.h"
#include <QPainter>
#include <QPainterPath>
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
}

void PumpWidget::setRunning(bool on)
{
    if (m_running == on) return;
    m_running = on;
    if (on)
        m_animTimer->start();
    else {
        m_animTimer->stop();
        m_speed = 0;
        m_rotation = 0;
        m_glowPhase = 0;
    }
    update();
}

void PumpWidget::setSpeed(qreal pct)
{
    m_speed = qBound(0.0, pct, 100.0);
    if (!m_running && m_speed > 0)
        setRunning(true);
    else if (m_speed == 0 && m_running)
        setRunning(false);
}

void PumpWidget::setRotation(qreal deg)
{
    m_rotation = deg;
    update();
}

void PumpWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int w = width();
    int h = height();
    int cx = w / 2, cy = h / 2 - 6;
    int r = qMin(w, h) / 2 - 14;

    // ---- 1. 运转光晕（呼吸效果） ----
    if (m_running) {
        qreal glowIntensity = 0.15 + 0.1 * qSin(m_glowPhase);
        hmi::drawGlowEffect(p, QPointF(cx, cy), r * 1.4,
                            QColor(30, 200, 80), glowIntensity);
    }

    // ---- 2. 进出口短管 ----
    p.setPen(Qt::NoPen);
    QLinearGradient stubGradH(0, cy - r * 0.25, 0, cy + r * 0.25);
    stubGradH.setColorAt(0.0, QColor(80, 85, 92));
    stubGradH.setColorAt(0.3, QColor(110, 116, 124));
    stubGradH.setColorAt(0.7, QColor(90, 95, 102));
    stubGradH.setColorAt(1.0, QColor(60, 64, 70));
    p.setBrush(stubGradH);

    // 左进口
    QRectF stubL(cx - r - 10, cy - r * 0.25, 10, r * 0.5);
    p.drawRoundedRect(stubL, 2, 2);
    // 右出口
    QRectF stubR(cx + r, cy - r * 0.25, 10, r * 0.5);
    p.drawRoundedRect(stubR, 2, 2);

    // ---- 3. 金属泵壳外环 ----
    QRadialGradient ringGrad(cx - r * 0.25, cy - r * 0.25, r * 1.1);
    ringGrad.setColorAt(0.85, QColor(130, 138, 148));
    ringGrad.setColorAt(0.92, QColor(100, 108, 118));
    ringGrad.setColorAt(0.97, QColor(70, 76, 84));
    ringGrad.setColorAt(1.0, QColor(50, 54, 60));
    p.setBrush(ringGrad);
    p.setPen(QPen(QColor(50, 54, 60), 2));
    p.drawEllipse(QPointF(cx, cy), r * 1.1, r * 1.1);

    // ---- 4. 泵体内腔 ----
    QRadialGradient bodyGrad(cx - r * 0.3, cy - r * 0.3, r);
    QColor base = m_running ? QColor(30, 140, 80) : QColor(80, 85, 90);
    bodyGrad.setColorAt(0.0, base.lighter(140));
    bodyGrad.setColorAt(0.4, base);
    bodyGrad.setColorAt(0.8, base.darker(130));
    bodyGrad.setColorAt(1.0, base.darker(170));
    p.setBrush(bodyGrad);
    p.setPen(QPen(base.darker(200), 2));
    p.drawEllipse(QPointF(cx, cy), r, r);

    // ---- 5. 转速弧形 ----
    if (m_running && m_speed > 0) {
        qreal span = m_speed / 100.0 * 270.0; // 0-270°
        QColor arcColor;
        if (m_speed < 30)      arcColor = QColor(40, 200, 60);
        else if (m_speed < 60) arcColor = QColor(255, 200, 50);
        else if (m_speed < 85) arcColor = QColor(255, 150, 30);
        else                   arcColor = QColor(240, 50, 50);
        hmi::drawSegmentedArc(p, QPointF(cx, cy), r + 10, 45, -span, 5, arcColor);
    }

    // ---- 6. 后弯离心叶轮 ----
    p.save();
    p.translate(cx, cy);
    p.rotate(m_rotation);

    QColor bladeColor = m_running ? QColor(220, 230, 240) : QColor(120, 125, 130);
    p.setBrush(bladeColor);
    p.setPen(Qt::NoPen);

    for (int i = 0; i < 3; ++i) {
        p.rotate(120);
        QPainterPath blade;
        qreal innerR = r * 0.2;
        qreal outerR = r * 0.75;
        // 后弯曲线：从内圈到外圈，向外弯曲
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

    // ---- 7. 中心六角轮毂 ----
    p.setBrush(Qt::NoBrush);
    p.setPen(Qt::NoPen);
    QRadialGradient hubGrad(cx - r * 0.08, cy - r * 0.08, r * 0.25);
    hubGrad.setColorAt(0.0, QColor(200, 205, 210));
    hubGrad.setColorAt(0.5, QColor(150, 155, 160));
    hubGrad.setColorAt(1.0, QColor(90, 95, 100));
    p.setBrush(hubGrad);
    p.setPen(QPen(QColor(70, 75, 80), 1));

    QPainterPath hex;
    qreal hr = r * 0.18;
    for (int i = 0; i < 6; ++i) {
        qreal angle = i * M_PI / 3.0 - M_PI / 6.0;
        QPointF pt(cx + hr * cos(angle), cy + hr * sin(angle));
        if (i == 0) hex.moveTo(pt);
        else hex.lineTo(pt);
    }
    hex.closeSubpath();
    p.drawPath(hex);

    // 中心轴点
    p.setBrush(QColor(180, 185, 190));
    p.setPen(QPen(QColor(90, 95, 100), 1));
    p.drawEllipse(QPointF(cx, cy), r * 0.08, r * 0.08);

    // ---- 标签 ----
    if (!m_label.isEmpty()) {
        p.setPen(m_running ? QColor(60, 220, 80) : QColor(180, 180, 180));
        p.setFont(QFont(font().family(), 10));
        p.drawText(QRect(0, cy + r + 12, w, 14), Qt::AlignHCenter | Qt::AlignTop, m_label);
    }
}
