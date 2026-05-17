#include "pump_widget.h"
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
    int cx = w / 2, cy = h / 2 - 4;
    int r = qMin(w, h) / 2 - 10;

    // 泵体外圈
    QRadialGradient bodyGrad(cx, cy, r);
    QColor base = m_running ? QColor(30, 140, 80) : QColor(80, 85, 90);
    bodyGrad.setColorAt(0.7, base);
    bodyGrad.setColorAt(1.0, base.darker(150));
    p.setBrush(bodyGrad);
    p.setPen(QPen(base.darker(200), 3));
    p.drawEllipse(QPoint(cx, cy), r, r);

    // 旋转叶轮
    p.save();
    p.translate(cx, cy);
    p.rotate(m_rotation);
    p.setBrush(m_running ? Qt::white : QColor(120, 125, 130));
    p.setPen(Qt::NoPen);
    for (int i = 0; i < 3; ++i) {
        p.rotate(120);
        QPainterPath blade;
        blade.moveTo(0, -r * 0.15);
        blade.lineTo(r * 0.5, -r * 0.45);
        blade.lineTo(r * 0.5, r * 0.45);
        blade.lineTo(0, r * 0.15);
        blade.closeSubpath();
        p.drawPath(blade);
    }
    p.restore();

    // 中心轴
    p.setBrush(QColor(180, 180, 180));
    p.setPen(QPen(QColor(100, 100, 100), 1));
    p.drawEllipse(QPointF(cx, cy), r * 0.3, r * 0.3);

    // 标签
    if (!m_label.isEmpty()) {
        p.setPen(m_running ? Qt::green : QColor(180, 180, 180));
        p.setFont(QFont(font().family(), 10));
        p.drawText(QRect(0, cy + r + 4, w, 14), Qt::AlignHCenter | Qt::AlignTop, m_label);
    }
}
