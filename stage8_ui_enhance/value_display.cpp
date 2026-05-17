#include "value_display.h"
#include "widget_render_helpers.h"
#include <QPainter>
#include <QPainterPath>

ValueDisplay::ValueDisplay(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(100, 70);

    m_flashTimer = new QTimer(this);
    m_flashTimer->setSingleShot(true);
    m_flashTimer->setInterval(180);
    connect(m_flashTimer, &QTimer::timeout, this, [this]() {
        m_flashActive = false;
        update();
    });

    m_alarmTimer = new QTimer(this);
    m_alarmTimer->setInterval(700);
    connect(m_alarmTimer, &QTimer::timeout, this, [this]() {
        m_alarmPulse = !m_alarmPulse;
        update();
    });
}

void ValueDisplay::setValue(qreal val)
{
    if (qFuzzyCompare(m_value, val)) return;
    m_prevValue = m_value;
    m_value = val;

    bool inAlarm = m_value < m_alarmLow || m_value > m_alarmHigh;
    if (inAlarm && !m_alarmTimer->isActive())
        m_alarmTimer->start();
    else if (!inAlarm && m_alarmTimer->isActive())
        m_alarmTimer->stop();

    // 闪烁效果
    m_flashActive = true;
    m_flashTimer->start();

    update();
}

void ValueDisplay::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int w = width(), h = height();

    // ---- 报警判定 ----
    bool isAlarm = m_value < m_alarmLow || m_value > m_alarmHigh;
    bool isWarn = !isAlarm && (m_value < m_alarmLow * 1.1 || m_value > m_alarmHigh * 0.9);

    QColor borderColor = QColor(80, 85, 90);
    if (isAlarm) borderColor = QColor(240, 50, 50);
    else if (isWarn) borderColor = QColor(255, 170, 30);

    // ---- 1. 报警光晕 ----
    if (isAlarm && m_alarmPulse) {
        QPainterPath glowPath;
        glowPath.addRoundedRect(2, 2, w - 4, h - 4, 8, 8);
        hmi::drawGlowEffect(p, QPointF(w / 2.0, h / 2.0),
                            qMax(w, h) * 0.6, QColor(240, 50, 50), 0.35);
    }

    // ---- 2. LCD 面板背景 ----
    p.setPen(QPen(borderColor, 2));
    QLinearGradient bgGrad(0, 0, 0, h);
    bgGrad.setColorAt(0.0, QColor(32, 36, 42));
    bgGrad.setColorAt(0.5, QColor(28, 31, 37));
    bgGrad.setColorAt(1.0, QColor(24, 27, 32));
    p.setBrush(bgGrad);
    p.drawRoundedRect(1, 1, w - 2, h - 2, 8, 8);

    // ---- 3. 标签 + 颜色强调条 ----
    QColor accentColor = isAlarm ? QColor(240, 50, 50)
                       : isWarn ? QColor(255, 170, 30)
                       : QColor(80, 85, 90);
    p.setPen(Qt::NoPen);
    p.setBrush(accentColor);
    p.drawRoundedRect(6, 6, 20, 3, 1, 1);

    p.setPen(QColor(150, 155, 165));
    QFont labelFont(font().family(), 9);
    labelFont.setWeight(QFont::Medium);
    p.setFont(labelFont);
    p.drawText(QRect(30, 2, w - 36, h / 4), Qt::AlignLeft | Qt::AlignVCenter, m_label);

    // ---- 4. 数值 ----
    QFont valFont("Consolas", h / 3);
    valFont.setBold(true);
    valFont.setStyleHint(QFont::Monospace);
    p.setFont(valFont);

    QColor valColor = Qt::white;
    if (m_flashActive) valColor = QColor(255, 255, 200);

    QString valStr = QString::number(m_value, 'f', m_decimals);
    QFontMetrics fm(valFont);
    int valW = fm.horizontalAdvance(valStr);
    QRectF valRect(w / 2 - valW / 2 - 4, h / 4, valW + 8, h / 2);

    hmi::drawTextWithShadow(p, valRect, Qt::AlignCenter, valStr,
                            valColor, QColor(0, 0, 0, 100));

    // ---- 5. 单位 ----
    if (!m_unit.isEmpty()) {
        p.setPen(QColor(130, 135, 145));
        QFont unitFont(font().family(), h / 6);
        p.setFont(unitFont);
        p.drawText(QRect(w / 2 + valW / 2 + 6, h / 4, w / 2 - valW / 2 - 8, h / 2),
                   Qt::AlignLeft | Qt::AlignVCenter, m_unit);
    }

    // ---- 6. 边框重新绘制（盖在背景之上） ----
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(borderColor, 2));
    p.drawRoundedRect(1, 1, w - 2, h - 2, 8, 8);
}
