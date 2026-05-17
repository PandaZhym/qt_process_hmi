#include "tank_widget.h"
#include "svg_assets.h"
#include "svg_renderer_pool.h"
#include "svg_render_helpers.h"
#include <QPainter>
#include <QPainterPath>
#include <QSvgRenderer>
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

    // 注册 SVG
    auto &pool = SvgRendererPool::instance();
    if (!pool.renderer("tank"))
        pool.add("tank", QByteArray(svg_assets::TANK_BODY));
}

void TankWidget::setLevel(qreal level)
{
    level = qBound(0.0, level, 100.0);
    if (qFuzzyCompare(m_level, level)) return;
    m_level = level;
    if (level > 0 && !m_surfaceTimer->isActive()) m_surfaceTimer->start();
    else if (level == 0 && m_surfaceTimer->isActive()) m_surfaceTimer->stop();
    update();
}

void TankWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int w = width(), h = height();
    int margin = w * 0.08;

    // SVG 罐体坐标映射（SVG viewBox 是 200x340）
    int svgX = margin;
    int svgY = margin + 18;
    int svgW = w - margin * 2;
    int svgH = h - 28 - margin * 2;
    QRectF svgRect(svgX, svgY, svgW, svgH);

    // 阴影
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 50));
    QPainterPath shadowPath;
    shadowPath.addRoundedRect(svgRect.translated(3, 3), svgW * 0.14, svgW * 0.14);
    p.drawPath(shadowPath);

    // ---- SVG 静态罐体 ----
    QSvgRenderer *tankSvg = SvgRendererPool::instance().renderer("tank");
    if (tankSvg)
        tankSvg->render(&p, svgRect);

    // ---- 动态液位覆盖（QPainter 分层叠加） ----
    // 有效显示区域（避开 SVG 的顶部法兰和底部排出管）
    qreal svgHeaderH = svgH * 0.02;
    qreal svgFooterH = svgH * 0.05;
    qreal innerTop = svgY + svgHeaderH;
    qreal innerBottom = svgY + svgH - svgFooterH;
    qreal innerHeight = innerBottom - innerTop;
    qreal viewPortLeft = svgX + svgW * 0.16;  // 观察窗后面

    qreal fillRatio = m_level / 100.0;
    int fillTop = innerBottom - (int)(innerHeight * fillRatio);

    QPainterPath fillPath;
    if (m_level > 0) {
        fillPath.moveTo(viewPortLeft, innerBottom);
        fillPath.lineTo(viewPortLeft, fillTop);
        // 表面波
        if (m_level > 0 && m_level < 99) {
            qreal amp = 2.0;
            qreal waveW = svgW * 0.65;
            for (int x = 0; x <= waveW; ++x) {
                qreal waveY = fillTop + amp * qSin((x / (waveW / 3.0)) * 2 * M_PI + m_surfacePhase);
                fillPath.lineTo(viewPortLeft + x, waveY);
            }
        } else {
            fillPath.lineTo(svgX + svgW - margin, fillTop);
        }
        fillPath.lineTo(svgX + svgW - margin, innerBottom);
        fillPath.closeSubpath();
    }

    // 裁剪液位到罐体圆角内
    QPainterPath tankClip;
    // 近似 SVG 圆角内框
    qreal clipRadius = svgW * 0.14;
    tankClip.addRoundedRect(svgX + 10, svgY + 10, svgW - 20, svgH - 20, clipRadius - 2, clipRadius - 2);
    fillPath = fillPath.intersected(tankClip);

    QColor liquidColor;
    bool isAlarm = m_level < m_alarmLow || m_level > m_alarmHigh;
    if (isAlarm)      liquidColor = QColor(220, 50, 50);
    else if (m_level > m_alarmHigh) liquidColor = QColor(255, 150, 30);
    else              liquidColor = QColor(30, 180, 220);

    QLinearGradient liquidGrad(0, fillTop, 0, innerBottom);
    liquidGrad.setColorAt(0.0, liquidColor.lighter(160));
    liquidGrad.setColorAt(0.2, liquidColor.lighter(130));
    liquidGrad.setColorAt(0.5, liquidColor);
    liquidGrad.setColorAt(0.8, liquidColor.darker(120));
    liquidGrad.setColorAt(1.0, liquidColor.darker(150));
    p.setPen(Qt::NoPen);
    p.setBrush(liquidGrad);
    p.drawPath(fillPath);

    // 玻璃反光
    if (m_level > 0) {
        QPainterPath reflPath;
        qreal reflW = svgW * 0.25;
        reflPath.addRect(viewPortLeft, fillTop, reflW, innerBottom - fillTop);
        reflPath = reflPath.intersected(tankClip).intersected(fillPath);
        p.setClipPath(reflPath);
        QLinearGradient reflGrad(reflPath.boundingRect().topLeft(),
                                  reflPath.boundingRect().bottomRight());
        reflGrad.setColorAt(0.0, QColor(255, 255, 255, 60));
        reflGrad.setColorAt(0.5, QColor(255, 255, 255, 0));
        p.setBrush(reflGrad);
        p.drawRect(reflPath.boundingRect());
        p.setClipping(false);
    }

    // 设定值指示
    if (m_setpoint >= 0 && m_setpoint <= 100) {
        int spY = innerBottom - (int)(innerHeight * m_setpoint / 100.0);
        spY = qBound((int)innerTop + 4, spY, (int)innerBottom - 4);
        p.setPen(QPen(QColor(255, 200, 50), 2));
        p.setBrush(QColor(255, 200, 50, 80));
        QPainterPath bracket;
        qreal bx = svgX + svgW - 6;
        bracket.moveTo(bx, spY - 6);
        bracket.lineTo(bx + 10, spY);
        bracket.lineTo(bx, spY + 6);
        bracket.closeSubpath();
        p.drawPath(bracket);

        QFont spFont = font();
        spFont.setPixelSize(10);
        p.setFont(spFont);
        p.setPen(QColor(255, 200, 50));
        p.drawText(QRectF(bx - 8, spY - 18, 50, 14), Qt::AlignLeft | Qt::AlignVCenter,
                   QString("SP %1%").arg(m_setpoint, 0, 'f', 1));
    }

    // 状态 LED（右上角）
    QColor ledColor = isAlarm ? QColor(255, 40, 40)
                    : (m_level > m_alarmHigh) ? QColor(255, 150, 30)
                    : QColor(40, 200, 60);
    svg_helpers::drawLEDIndicator(p, QPointF(svgX + svgW * 0.9, svgY + svgHeaderH * 2), 5, true, ledColor);

    // 百分比文字
    QFont pctFont = font();
    pctFont.setPixelSize(innerHeight / 7);
    pctFont.setBold(true);
    p.setFont(pctFont);
    svg_helpers::drawTextWithShadow(p,
        QRectF(viewPortLeft, innerTop + innerHeight/2 - innerHeight/8,
               svgW * 0.7, innerHeight/4),
        Qt::AlignCenter, QString("%1%").arg(m_level, 0, 'f', 1),
        Qt::white, QColor(0, 0, 0, 120));

    // 标签
    p.setFont(QFont(font().family(), 10));
    svg_helpers::drawTextWithShadow(p,
        QRectF(svgX, svgY + svgH + 2, svgW, 18),
        Qt::AlignHCenter | Qt::AlignTop, m_label,
        QColor(210, 215, 220), QColor(0, 0, 0, 80));
}
