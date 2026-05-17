#include "value_display.h"
#include "svg_assets.h"
#include "svg_renderer_pool.h"
#include "svg_render_helpers.h"
#include <QPainter>
#include <QPainterPath>
#include <QSvgRenderer>

ValueDisplay::ValueDisplay(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(100, 70);

    m_flashTimer = new QTimer(this);
    m_flashTimer->setSingleShot(true);
    m_flashTimer->setInterval(180);
    connect(m_flashTimer, &QTimer::timeout, this, [this]() {
        m_flashActive = false; update();
    });

    m_alarmTimer = new QTimer(this);
    m_alarmTimer->setInterval(700);
    connect(m_alarmTimer, &QTimer::timeout, this, [this]() {
        m_alarmPulse = !m_alarmPulse; update();
    });

    auto &pool = SvgRendererPool::instance();
    if (!pool.renderer("panel"))
        pool.add("panel", QByteArray(svg_assets::VALUE_PANEL));
}

void ValueDisplay::setValue(qreal val)
{
    if (qFuzzyCompare(m_value, val)) return;
    m_value = val;

    bool inAlarm = m_value < m_alarmLow || m_value > m_alarmHigh;
    if (inAlarm && !m_alarmTimer->isActive()) m_alarmTimer->start();
    else if (!inAlarm && m_alarmTimer->isActive()) m_alarmTimer->stop();

    m_flashActive = true;
    m_flashTimer->start();
    update();
}

void ValueDisplay::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int w = width(), h = height();

    bool isAlarm = m_value < m_alarmLow || m_value > m_alarmHigh;
    bool isWarn = !isAlarm && (m_value < m_alarmLow * 1.1 || m_value > m_alarmHigh * 0.9);

    // 报警光晕
    if (isAlarm && m_alarmPulse) {
        svg_helpers::drawGlowEffect(p, QPointF(w / 2.0, h / 2.0),
                                    qMax(w, h) * 0.5, QColor(240, 50, 50), 0.35);
    }

    // ---- SVG 仪表面板 ----
    QSvgRenderer *panelSvg = SvgRendererPool::instance().renderer("panel");
    if (panelSvg) panelSvg->render(&p, QRectF(0, 0, w, h));

    // ---- 动态报警边框颜色 ----
    QColor borderColor = isAlarm ? QColor(240, 50, 50)
                       : isWarn ? QColor(255, 170, 30)
                       : QColor(80, 85, 90);
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(borderColor, 2.5));
    p.drawRoundedRect(2, 2, w - 4, h - 4, 10, 10);

    // 标签强调条
    QColor accentColor = isAlarm ? QColor(240, 50, 50)
                       : isWarn ? QColor(255, 170, 30)
                       : QColor(80, 85, 90);
    p.setPen(Qt::NoPen);
    p.setBrush(accentColor);
    p.drawRoundedRect(12, 10, 24, 3, 1, 1);

    p.setPen(QColor(150, 155, 165));
    QFont labelFont(font().family(), 9);
    labelFont.setWeight(QFont::Medium);
    p.setFont(labelFont);
    p.drawText(QRect(38, 4, w - 48, h / 4), Qt::AlignLeft | Qt::AlignVCenter, m_label);

    // 数值
    QFont valFont("Consolas", h / 3);
    valFont.setBold(true);
    valFont.setStyleHint(QFont::Monospace);
    p.setFont(valFont);
    QColor valColor = m_flashActive ? QColor(255, 255, 200) : Qt::white;
    QString valStr = QString::number(m_value, 'f', m_decimals);
    QFontMetrics fm(valFont);
    int valW = fm.horizontalAdvance(valStr);
    svg_helpers::drawTextWithShadow(p,
        QRectF(w / 2 - valW / 2 - 4, h / 4, valW + 8, h / 2),
        Qt::AlignCenter, valStr, valColor, QColor(0, 0, 0, 100));

    // 单位
    if (!m_unit.isEmpty()) {
        p.setPen(QColor(130, 135, 145));
        p.setFont(QFont(font().family(), h / 6));
        p.drawText(QRect(w / 2 + valW / 2 + 6, h / 4, w / 2 - valW / 2 - 8, h / 2),
                   Qt::AlignLeft | Qt::AlignVCenter, m_unit);
    }
}
