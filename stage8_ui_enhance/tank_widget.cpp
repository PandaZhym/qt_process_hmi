#include "tank_widget.h"
#include "widget_render_helpers.h"
#include <QPainter>
#include <QPainterPath>
#include <QtMath>

TankWidget::TankWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(100, 160);
    m_anim = new QPropertyAnimation(this, "level", this);
    m_anim->setDuration(600);
    m_anim->setEasingCurve(QEasingCurve::OutCubic);

    m_surfaceTimer = new QTimer(this);
    m_surfaceTimer->setInterval(50);
    connect(m_surfaceTimer, &QTimer::timeout, this, [this]() {
        m_surfacePhase += 0.15;
        if (m_surfacePhase > 2 * M_PI) m_surfacePhase -= 2 * M_PI;
        update();
    });

    m_alarmGlowTimer = new QTimer(this);
    m_alarmGlowTimer->setInterval(600);
    connect(m_alarmGlowTimer, &QTimer::timeout, this, [this]() {
        m_alarmGlowOn = !m_alarmGlowOn;
        update();
    });
}

void TankWidget::setLevel(qreal level)
{
    level = qBound(0.0, level, 100.0);
    if (qFuzzyCompare(m_level, level)) return;
    m_level = level;

    if (level > 0 && !m_surfaceTimer->isActive())
        m_surfaceTimer->start();
    else if (level == 0 && m_surfaceTimer->isActive())
        m_surfaceTimer->stop();

    update();
}

static bool s_alarmGlowActive = false;

void TankWidget::setAlarm(bool active)
{
    if (m_alarm == active) return;
    m_alarm = active;
    if (active && !m_alarmGlowTimer->isActive()) {
        s_alarmGlowActive = true;
        m_alarmGlowTimer->start();
    } else if (!active && m_alarmGlowTimer->isActive()) {
        s_alarmGlowActive = false;
        m_alarmGlowTimer->stop();
        m_alarmGlowOn = false;
    }
    update();
}

void TankWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int w = width();
    int h = height();
    int margin = w * 0.15;
    int rulerWidth = w * 0.14;

    int tankLeft = margin + rulerWidth;
    int tankRight = w - margin;
    int tankTop = margin + 18;
    int tankBottom = h - 28 - margin;
    int tankWidth = tankRight - tankLeft;
    int tankHeight = tankBottom - tankTop;
    qreal tankRadius = tankWidth / 3.0;

    QRectF tankRect(tankLeft, tankTop, tankWidth, tankHeight);

    // ---- 1. 阴影 ----
    QPainterPath shadowPath;
    shadowPath.addRoundedRect(tankRect.translated(3, 3), tankRadius, tankRadius);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 60));
    p.drawPath(shadowPath);

    // ---- 2. 金属外框 ----
    hmi::drawMetallicFrame(p, tankRect.adjusted(-3, -3, 3, 3),
                           tankRadius + 2,
                           QColor(90, 98, 110), true);

    // 螺栓
    qreal boltR = 4;
    hmi::drawBolt(p, QPointF(tankLeft - 8, tankTop + 12), boltR);
    hmi::drawBolt(p, QPointF(tankRight + 8, tankTop + 12), boltR);
    hmi::drawBolt(p, QPointF(tankLeft - 8, tankBottom - 12), boltR);
    hmi::drawBolt(p, QPointF(tankRight + 8, tankBottom - 12), boltR);

    // ---- 3. 罐内暗井 ----
    p.setPen(Qt::NoPen);
    QLinearGradient wellGrad(tankRect.topLeft(), tankRect.topRight());
    wellGrad.setColorAt(0.0, QColor(25, 30, 36));
    wellGrad.setColorAt(0.5, QColor(20, 24, 30));
    wellGrad.setColorAt(1.0, QColor(25, 30, 36));
    p.setBrush(wellGrad);
    p.drawRoundedRect(tankRect, tankRadius, tankRadius);

    // ---- 4. 刻度尺 ----
    hmi::drawRulerScale(p, QRectF(margin, tankTop, rulerWidth - 4, tankHeight),
                        Qt::Vertical, 5, QColor(140, 145, 155, 80));

    // ---- 5. 液位填充 ----
    qreal fillRatio = m_level / 100.0;
    int fillTop = tankBottom - (int)(tankHeight * fillRatio);

    // 液位区域路径（带表面波）
    QPainterPath fillPath;
    if (m_level > 0) {
        fillPath.moveTo(tankLeft, tankBottom);
        fillPath.lineTo(tankLeft, fillTop);
        // 表面波
        if (m_level > 0 && m_level < 99) {
            qreal amp = 2.5;
            qreal waveLen = tankWidth / 3.0;
            for (int x = 0; x <= tankWidth; ++x) {
                qreal xf = tankLeft + x;
                qreal waveY = fillTop + amp * qSin((x / waveLen) * 2 * M_PI + m_surfacePhase);
                if (x == 0) fillPath.lineTo(xf, waveY);
                else fillPath.lineTo(xf, waveY);
            }
        } else {
            fillPath.lineTo(tankRight, fillTop);
        }
        fillPath.lineTo(tankRight, tankBottom);
        fillPath.closeSubpath();
    }

    // 裁剪到罐体圆角内
    QPainterPath tankClip;
    tankClip.addRoundedRect(tankRect, tankRadius, tankRadius);
    fillPath = fillPath.intersected(tankClip);

    QColor liquidColor;
    if (m_alarm || m_level < m_alarmLow)
        liquidColor = QColor(220, 50, 50);
    else if (m_level > m_alarmHigh)
        liquidColor = QColor(255, 150, 30);
    else
        liquidColor = QColor(30, 180, 220);

    QLinearGradient liquidGrad(0, fillTop, 0, tankBottom);
    liquidGrad.setColorAt(0.0, liquidColor.lighter(160));
    liquidGrad.setColorAt(0.2, liquidColor.lighter(140));
    liquidGrad.setColorAt(0.5, liquidColor);
    liquidGrad.setColorAt(0.8, liquidColor.darker(120));
    liquidGrad.setColorAt(1.0, liquidColor.darker(150));
    p.setPen(Qt::NoPen);
    p.setBrush(liquidGrad);
    p.drawPath(fillPath);

    // ---- 6. 玻璃反光 ----
    if (m_level > 0) {
        QPainterPath reflPath;
        qreal reflWidth = tankWidth * 0.35;
        reflPath.addRect(tankLeft, fillTop, reflWidth, tankBottom - fillTop);
        reflPath = reflPath.intersected(tankClip);
        // 限制在液位区域内
        reflPath = reflPath.intersected(fillPath);
        hmi::drawGlassReflection(p, reflPath, Qt::Vertical);
    }

    // ---- 7. 罐体轮廓 ----
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(QColor(160, 170, 180), 2));
    p.drawRoundedRect(tankRect, tankRadius, tankRadius);

    // ---- 8. 设定值指示 ----
    if (m_setpoint >= 0 && m_setpoint <= 100) {
        int spY = tankBottom - (int)(tankHeight * m_setpoint / 100.0);
        spY = qBound(tankTop + 4, spY, tankBottom - 4);

        // 三角括号标记
        p.setPen(QPen(QColor(255, 200, 50), 2));
        p.setBrush(QColor(255, 200, 50, 80));
        QPainterPath bracket;
        int bx = tankRight + 2;
        bracket.moveTo(bx, spY - 6);
        bracket.lineTo(bx + 8, spY);
        bracket.lineTo(bx, spY + 6);
        bracket.closeSubpath();
        p.drawPath(bracket);

        // SP 文字
        QFont spFont = font();
        spFont.setPixelSize(10);
        p.setFont(spFont);
        p.setPen(QColor(255, 200, 50));
        p.drawText(QRectF(bx - 2, spY - 18, 40, 14), Qt::AlignLeft | Qt::AlignVCenter,
                   QString("SP %1%").arg(m_setpoint, 0, 'f', 1));
    }

    // ---- 9. 状态 LED ----
    bool ledOn = true;
    QColor ledColor;
    if (m_alarm || m_level < m_alarmLow)
        ledColor = m_alarmGlowOn ? QColor(255, 40, 40) : QColor(180, 30, 30);
    else if (m_level > m_alarmHigh)
        ledColor = QColor(255, 150, 30);
    else
        ledColor = QColor(40, 200, 60);
    hmi::drawLEDIndicator(p, QPointF(tankRight + 12, tankTop - 10), 5, ledOn, ledColor);

    // ---- 10. 百分比文字 ----
    QFont pctFont = font();
    pctFont.setPixelSize(tankHeight / 7);
    pctFont.setBold(true);
    p.setFont(pctFont);
    QString pctText = QString("%1%").arg(m_level, 0, 'f', 1);
    hmi::drawTextWithShadow(p, QRectF(tankLeft, tankTop + tankHeight / 2 - tankHeight / 8,
                                       tankWidth, tankHeight / 4),
                            Qt::AlignCenter, pctText, Qt::white, QColor(0, 0, 0, 120));

    // ---- 11. 底部标签 ----
    p.setFont(QFont(font().family(), 10));
    hmi::drawTextWithShadow(p, QRectF(tankLeft, tankBottom + 4, tankWidth, 20),
                            Qt::AlignHCenter | Qt::AlignTop, m_label,
                            QColor(210, 215, 220), QColor(0, 0, 0, 80));
}
