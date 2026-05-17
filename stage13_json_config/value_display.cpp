#include "value_display.h"
#include <QPainter>
#include <QPainterPath>

ValueDisplay::ValueDisplay(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(100, 70);
}

void ValueDisplay::setValue(qreal val)
{
    if (qFuzzyCompare(m_value, val)) return;
    m_value = val;
    update();
}

void ValueDisplay::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int w = width(), h = height();

    // 报警边框
    QColor borderColor = QColor(80, 85, 90);
    if (m_value < m_alarmLow || m_value > m_alarmHigh)
        borderColor = QColor(220, 50, 50);
    else if (m_value < m_alarmLow * 1.1 || m_value > m_alarmHigh * 0.9)
        borderColor = QColor(255, 170, 30);

    p.setBrush(QColor(38, 42, 48));
    p.setPen(QPen(borderColor, 2));
    p.drawRoundedRect(1, 1, w - 2, h - 2, 8, 8);

    // 标签（顶部）
    p.setPen(QColor(160, 165, 170));
    p.setFont(QFont(font().family(), 9));
    p.drawText(QRect(4, 2, w - 8, h / 4), Qt::AlignLeft | Qt::AlignBottom, m_label);

    // 数值（中间）
    p.setPen(Qt::white);
    QFont valFont(font().family(), h / 3);
    valFont.setBold(true);
    p.setFont(valFont);
    QString valStr = QString::number(m_value, 'f', m_decimals);
    p.drawText(QRect(4, h / 4, w - 8, h / 2), Qt::AlignHCenter | Qt::AlignVCenter, valStr);

    // 单位（数值右边）
    if (!m_unit.isEmpty()) {
        p.setPen(QColor(140, 145, 150));
        p.setFont(QFont(font().family(), h / 6));
        QFontMetrics fm(valFont);
        int valW = fm.horizontalAdvance(valStr);
        p.drawText(QRect(w/2 + valW/2 + 2, h/4, w/2 - valW/2 - 4, h/2),
                   Qt::AlignLeft | Qt::AlignVCenter, m_unit);
    }
}
