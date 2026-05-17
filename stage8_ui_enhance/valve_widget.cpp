#include "valve_widget.h"
#include "widget_render_helpers.h"
#include <QPainter>
#include <QPainterPath>
#include <QtMath>

ValveWidget::ValveWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(60, 60);
    m_anim = new QPropertyAnimation(this, "opening", this);
    m_anim->setDuration(400);
    m_anim->setEasingCurve(QEasingCurve::OutCubic);
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

    QRectF bodyRect(cx - sz / 2.0, cy - sz / 2.0, sz, sz);

    // ---- 1. 金属阀体 ----
    hmi::drawMetallicFrame(p, bodyRect, sz / 5.0,
                           QColor(100, 108, 118), true);

    // ---- 2. 左右法兰板 ----
    int flangeW = sz * 0.2;
    int flangeH = sz * 0.7;
    QRectF flangeL(bodyRect.left() - flangeW * 0.8, cy - flangeH / 2.0, flangeW, flangeH);
    QRectF flangeR(bodyRect.right() - flangeW * 0.2, cy - flangeH / 2.0, flangeW, flangeH);

    hmi::drawMetallicFrame(p, flangeL, 3, QColor(85, 92, 100), false);
    hmi::drawMetallicFrame(p, flangeR, 3, QColor(85, 92, 100), false);

    // 法兰螺栓
    hmi::drawBolt(p, QPointF(flangeL.center().x(), flangeL.top() + 6), 3);
    hmi::drawBolt(p, QPointF(flangeL.center().x(), flangeL.bottom() - 6), 3);
    hmi::drawBolt(p, QPointF(flangeR.center().x(), flangeR.top() + 6), 3);
    hmi::drawBolt(p, QPointF(flangeR.center().x(), flangeR.bottom() - 6), 3);

    // ---- 3. 流通背景（阀体内水平管） ----
    bool isOpen = m_opening > 5;
    QColor pipeColor = isOpen ? QColor(30, 180, 220) : QColor(50, 55, 60);
    p.setPen(Qt::NoPen);
    p.setBrush(pipeColor.darker(130));
    QRectF pipeInner(bodyRect.left() + 8, cy - sz * 0.08, bodyRect.width() - 16, sz * 0.16);
    p.drawRoundedRect(pipeInner, 2, 2);

    // ---- 4. 扇形开度指示 ----
    qreal arcRadius = sz * 0.32;
    qreal openAngle = m_opening / 100.0 * 90.0; // 0° → 90°

    // 颜色计算
    QColor arcColor;
    if (m_opening < 10)      arcColor = QColor(220, 50, 50);
    else if (m_opening < 40) arcColor = QColor(255, 170, 30);
    else if (m_opening < 70) arcColor = QColor(255, 200, 50);
    else                     arcColor = QColor(40, 200, 60);

    if (openAngle > 1) {
        // 背景弧（暗灰）
        hmi::drawSegmentedArc(p, QPointF(cx, cy), arcRadius,
                              90, -90, sz * 0.12, QColor(45, 50, 55), true);
        // 开度弧
        hmi::drawSegmentedArc(p, QPointF(cx, cy), arcRadius,
                              90, -openAngle, sz * 0.12, arcColor, true);
    } else {
        // 全关：只画暗灰背景
        hmi::drawSegmentedArc(p, QPointF(cx, cy), arcRadius,
                              90, -90, sz * 0.12, QColor(45, 50, 55), true);
    }

    // ---- 5. 中心阀杆旋钮 ----
    QPointF knobCenter(cx, cy);
    qreal knobR = sz * 0.15;
    QRadialGradient knobGrad(knobCenter - QPointF(knobR * 0.3, knobR * 0.3), knobR);
    knobGrad.setColorAt(0.0, QColor(210, 215, 220));
    knobGrad.setColorAt(0.5, QColor(150, 155, 160));
    knobGrad.setColorAt(1.0, QColor(90, 96, 102));
    p.setBrush(knobGrad);
    p.setPen(QPen(QColor(70, 75, 80), 1));
    p.drawEllipse(knobCenter, knobR, knobR);

    // 十字指示线
    p.setPen(QPen(QColor(60, 65, 70), 1.5));
    qreal crossLen = knobR * 0.7;
    p.drawLine(QPointF(cx - crossLen, cy), QPointF(cx + crossLen, cy));
    p.drawLine(QPointF(cx, cy - crossLen), QPointF(cx, cy + crossLen));

    // ---- 6. 开度百分比 ----
    QFont pctFont = font();
    pctFont.setPixelSize(sz / 5);
    pctFont.setBold(true);
    p.setFont(pctFont);
    hmi::drawTextWithShadow(p, QRectF(cx - sz / 2, cy - sz / 4, sz, sz / 2),
                            Qt::AlignCenter,
                            QString("%1%").arg(m_opening, 0, 'f', 0),
                            Qt::white, QColor(0, 0, 0, 100));

    // ---- 标签 ----
    if (!m_label.isEmpty()) {
        p.setFont(QFont(font().family(), 9));
        p.setPen(QColor(200, 200, 200));
        p.drawText(QRect(0, cy + sz / 2 + 4, w, 14),
                   Qt::AlignHCenter | Qt::AlignTop, m_label);
    }
}
