#include "alarm_list_widget.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QFontMetrics>
#include <QtMath>
#include <algorithm>

AlarmListWidget::AlarmListWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumHeight(120);
    setMouseTracking(true);

    m_blinkTimer = new QTimer(this);
    m_blinkTimer->setInterval(500);
    connect(m_blinkTimer, &QTimer::timeout, this, [this]() {
        m_blinkPhase += 0.5;
        update();
    });
    m_blinkTimer->start();
}

void AlarmListWidget::setAlarmRecords(const QVector<AlarmRecord> &records)
{
    m_records = records;
    // 最新在前
    std::sort(m_records.begin(), m_records.end(),
              [](const AlarmRecord &a, const AlarmRecord &b) {
                  return a.timestamp > b.timestamp;
              });
    update();
}

void AlarmListWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int w = width();
    int h = height();

    // 背景
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(26, 29, 35));
    p.drawRoundedRect(0, 0, w, h, 8, 8);

    if (m_records.isEmpty()) {
        p.setPen(QColor(100, 105, 110));
        p.setFont(QFont(font().family(), 10));
        p.drawText(QRect(0, 0, w, h), Qt::AlignCenter, "无活跃报警");
        return;
    }

    // 列表标题
    p.setPen(QColor(140, 145, 155));
    QFont headerFont(font().family(), 9);
    headerFont.setWeight(QFont::Medium);
    p.setFont(headerFont);
    int y = PADDING;
    int colX[6];
    colX[0] = PADDING + 4;                    // 色标
    colX[1] = PADDING + 24;                   // 标签名
    colX[2] = colX[1] + 120;                  // 类型
    colX[3] = colX[2] + 50;                   // 设定值
    colX[4] = colX[3] + 60;                   // 当前值
    colX[5] = colX[4] + 55;                   // 时间
    int colW[6] = { 14, 116, 46, 56, 50, 150 };

    p.drawText(QRect(colX[1], y, colW[1], ROW_HEIGHT), Qt::AlignLeft | Qt::AlignVCenter, "标签");
    p.drawText(QRect(colX[2], y, colW[2], ROW_HEIGHT), Qt::AlignLeft | Qt::AlignVCenter, "级别");
    p.drawText(QRect(colX[3], y, colW[3], ROW_HEIGHT), Qt::AlignLeft | Qt::AlignVCenter, "限值");
    p.drawText(QRect(colX[4], y, colW[4], ROW_HEIGHT), Qt::AlignLeft | Qt::AlignVCenter, "当前值");
    p.drawText(QRect(colX[5], y, colW[5], ROW_HEIGHT), Qt::AlignLeft | Qt::AlignVCenter, "时间");

    QFont rowFont(font().family(), 10);
    p.setFont(rowFont);

    int visibleRows = (h - ROW_HEIGHT - PADDING * 2) / ROW_HEIGHT;
    visibleRows = qMin(visibleRows, (int)m_records.size());

    for (int i = 0; i < visibleRows; ++i) {
        const auto &rec = m_records[i];
        int rowY = PADDING + ROW_HEIGHT + i * ROW_HEIGHT;
        QRect rowRect(4, rowY, w - 8, ROW_HEIGHT - 2);

        // 行背景
        bool isUnacked = !rec.acknowledged;
        bool blinkOn = (int)(m_blinkPhase) % 2 == 0;

        QColor rowBg;
        if (isUnacked && rec.active && blinkOn) {
            rowBg = QColor(180, 40, 40, 60);           // 活跃未确认 → 红色闪烁
        } else if (isUnacked && rec.active && !blinkOn) {
            rowBg = QColor(80, 40, 40, 40);
        } else if (isUnacked && !rec.active && blinkOn) {
            rowBg = QColor(160, 100, 20, 50);           // 已恢复未确认 → 橙色闪烁
        } else if (isUnacked && !rec.active && !blinkOn) {
            rowBg = QColor(80, 50, 20, 30);
        } else if (i == m_hoveredRow) {
            rowBg = QColor(255, 255, 255, 15);
        } else if (!rec.active) {
            rowBg = QColor(60, 60, 65, 30);             // 已恢复已确认 → 暗灰
        } else {
            rowBg = QColor(40, 42, 48);
        }

        p.setPen(Qt::NoPen);
        p.setBrush(rowBg);
        p.drawRoundedRect(rowRect, 4, 4);

        // 色标
        QColor severityColor;
        switch (rec.type) {
            case AlarmType::HH: severityColor = QColor(240, 50, 50); break;
            case AlarmType::H:  severityColor = QColor(255, 150, 30); break;
            case AlarmType::L:  severityColor = QColor(255, 170, 30); break;
            case AlarmType::LL: severityColor = QColor(220, 100, 30); break;
        }
        p.setBrush(severityColor);
        p.drawRoundedRect(colX[0], rowY + 8, 8, ROW_HEIGHT - 18, 3, 3);

        // 文本颜色
        QColor textColor = rec.active ? QColor(220, 225, 230) : QColor(140, 145, 150);
        p.setPen(textColor);

        // 标签名
        p.drawText(QRect(colX[1], rowY, colW[1], ROW_HEIGHT - 2),
                   Qt::AlignLeft | Qt::AlignVCenter, rec.tagName);

        // 报警类型
        QColor typeColor = (rec.type == AlarmType::HH || rec.type == AlarmType::LL)
                         ? QColor(240, 60, 60) : QColor(255, 170, 30);
        p.setPen(typeColor);
        QFont typeFont(font().family(), 10, QFont::Bold);
        p.setFont(typeFont);
        p.drawText(QRect(colX[2], rowY, colW[2], ROW_HEIGHT - 2),
                   Qt::AlignLeft | Qt::AlignVCenter, alarmTypeName(rec.type));
        p.setFont(rowFont);

        // 限值
        p.setPen(textColor);
        p.drawText(QRect(colX[3], rowY, colW[3], ROW_HEIGHT - 2),
                   Qt::AlignLeft | Qt::AlignVCenter,
                   QString::number(rec.limit, 'f', 1));

        // 当前值
        p.setPen(rec.active ? QColor(255, 255, 200) : textColor);
        p.drawText(QRect(colX[4], rowY, colW[4], ROW_HEIGHT - 2),
                   Qt::AlignLeft | Qt::AlignVCenter,
                   QString::number(rec.actualValue, 'f', 1));

        // 时间
        p.setPen(textColor);
        p.drawText(QRect(colX[5], rowY, colW[5], ROW_HEIGHT - 2),
                   Qt::AlignLeft | Qt::AlignVCenter,
                   rec.timestamp.toString("hh:mm:ss"));

        // 确认状态（4 种状态）
        QRect statusRect(colX[5] + colW[5] + 4, rowY, 60, ROW_HEIGHT - 2);
        if (!rec.active && rec.acknowledged) {
            p.setPen(QColor(100, 180, 100));
            p.drawText(statusRect, Qt::AlignLeft | Qt::AlignVCenter, "√ 已恢复");
        } else if (!rec.active && !rec.acknowledged) {
            p.setPen(QColor(255, 170, 30));
            p.drawText(statusRect, Qt::AlignLeft | Qt::AlignVCenter, "▲ 待确认");
        } else if (rec.active && rec.acknowledged) {
            p.setPen(QColor(255, 200, 50));
            p.drawText(statusRect, Qt::AlignLeft | Qt::AlignVCenter, "已确认");
        } else {
            p.setPen(QColor(240, 80, 80));
            p.drawText(statusRect, Qt::AlignLeft | Qt::AlignVCenter, "● 未确认");
        }
    }
}

void AlarmListWidget::mousePressEvent(QMouseEvent *event)
{
    int y = event->pos().y();
    int row = (y - ROW_HEIGHT - PADDING) / ROW_HEIGHT;
    if (row >= 0 && row < m_records.size()) {
        const auto &rec = m_records[row];
        if (!rec.acknowledged)
            emit acknowledgeRequested(rec.id);
    }
}

void AlarmListWidget::mouseMoveEvent(QMouseEvent *event)
{
    int y = event->pos().y();
    int row = (y - ROW_HEIGHT - PADDING) / ROW_HEIGHT;
    if (row >= 0 && row < m_records.size()) {
        if (m_hoveredRow != row) {
            m_hoveredRow = row;
            update();
        }
    } else if (m_hoveredRow != -1) {
        m_hoveredRow = -1;
        update();
    }
}
