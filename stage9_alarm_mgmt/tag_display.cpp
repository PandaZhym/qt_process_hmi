#include "tag_display.h"
#include <QPainter>

TagDisplay::TagDisplay(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(120, 80);
}

void TagDisplay::setValue(double val)
{
    m_value = val;
    update();
}

void TagDisplay::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int w = width(), h = height();

    // 背景
    QColor border = m_alarmActive ? QColor(240, 50, 50) : QColor(80, 85, 90);
    p.setBrush(QColor(32, 36, 42));
    p.setPen(QPen(border, 2));
    p.drawRoundedRect(1, 1, w - 2, h - 2, 6, 6);

    // 标签
    p.setPen(QColor(150, 155, 165));
    p.setFont(QFont(font().family(), 9));
    p.drawText(QRect(6, 2, w - 12, h / 4), Qt::AlignLeft | Qt::AlignVCenter, m_tagName);

    // 数值
    p.setPen(Qt::white);
    QFont valFont(font().family(), h / 3, QFont::Bold);
    p.setFont(valFont);
    QString valStr = QString::number(m_value, 'f', 1);
    p.drawText(QRect(6, h / 4, w - 12, h / 2), Qt::AlignCenter, valStr);

    // 单位
    if (!m_unit.isEmpty()) {
        p.setPen(QColor(130, 135, 145));
        p.setFont(QFont(font().family(), h / 6));
        p.drawText(QRect(6, h / 4 + h / 3, w - 12, h / 4), Qt::AlignCenter, m_unit);
    }
}
