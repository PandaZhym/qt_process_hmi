#include "tank_widget.h"
#include <QPainter>
#include <QPainterPath>
#include <QtMath>

TankWidget::TankWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(80, 160);
    m_anim = new QPropertyAnimation(this, "level", this);
    m_anim->setDuration(600);
    m_anim->setEasingCurve(QEasingCurve::OutCubic);
}

void TankWidget::setLevel(qreal level)
{
    level = qBound(0.0, level, 100.0);
    if (qFuzzyCompare(m_level, level)) return;
    m_level = level;
    update();
}

void TankWidget::setAlarm(bool active)
{
    if (m_alarm == active) return;
    m_alarm = active;
    update();
}

void TankWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int w = width();
    int h = height();
    int margin = w * 0.12;

    // 罐体区域
    int tankLeft = margin;
    int tankRight = w - margin;
    int tankTop = margin + 18;
    int tankBottom = h - 28 - margin;
    int tankWidth = tankRight - tankLeft;
    int tankHeight = tankBottom - tankTop;
    int radius = tankWidth / 3;

    // ---- 背景网格 ----
    p.setPen(QPen(QColor(50, 55, 60), 1, Qt::DotLine));
    int numLines = 8;
    for (int i = 1; i < numLines; ++i) {
        int y = tankTop + tankHeight * i / numLines;
        p.drawLine(tankLeft + radius, y, tankRight - radius, y);
    }

    // ---- 液位 ----
    qreal fillRatio = m_level / 100.0;
    int fillTop = tankBottom - (int)(tankHeight * fillRatio);

    QPainterPath fillPath;
    fillPath.addRoundedRect(tankLeft, fillTop, tankWidth, tankBottom - fillTop, radius, radius);
    // 超过一半用圆角矩形，否则裁剪底部
    if (fillTop > tankTop + radius) {
        QPainterPath tankPath;
        tankPath.addRoundedRect(tankLeft, tankTop, tankWidth, tankHeight, radius, radius);
        fillPath = fillPath.intersected(tankPath);
    }

    // 液位颜色：低警告=红, 高警告=橙, 正常=蓝绿渐变
    QColor liquidColor;
    if (m_alarm || m_level < m_alarmLow)
        liquidColor = QColor(220, 50, 50);
    else if (m_level > m_alarmHigh)
        liquidColor = QColor(255, 150, 30);
    else
        liquidColor = QColor(30, 180, 220);

    QLinearGradient liquidGrad(0, fillTop, 0, tankBottom);
    liquidGrad.setColorAt(0, liquidColor.lighter(130));
    liquidGrad.setColorAt(1, liquidColor);

    p.setPen(Qt::NoPen);
    p.setBrush(liquidGrad);
    p.drawPath(fillPath);

    // ---- 罐体轮廓 ----
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(QColor(140, 150, 160), 2));
    p.drawRoundedRect(tankLeft, tankTop, tankWidth, tankHeight, radius, radius);

    // ---- 设定值指示线 ----
    if (m_setpoint >= 0) {
        int spY = tankBottom - (int)(tankHeight * m_setpoint / 100.0);
        p.setPen(QPen(QColor(255, 200, 50), 2, Qt::DashLine));
        p.drawLine(tankLeft - 4, spY, tankRight + 4, spY);
    }

    // ---- 百分比文字 ----
    p.setPen(Qt::white);
    QFont fnt = font();
    fnt.setPixelSize(tankHeight / 6);
    fnt.setBold(true);
    p.setFont(fnt);
    p.drawText(QRect(0, tankTop + tankHeight/2 - tankHeight/8, w, tankHeight/4),
               Qt::AlignCenter, QString("%1%").arg(m_level, 0, 'f', 1));

    // ---- 底部标签 ----
    p.setFont(QFont(font().family(), 10));
    p.setPen(QColor(200, 200, 200));
    p.drawText(QRect(0, tankBottom + 4, w, 20), Qt::AlignHCenter | Qt::AlignTop, m_label);
}
