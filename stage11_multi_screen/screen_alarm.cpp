#include "screen_alarm.h"
#include "sim_data_manager.h"
#include "nav_bar.h"
#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QtMath>

ScreenAlarm::ScreenAlarm(QWidget *parent) : HmiScreen(parent)
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 8, 12, 8);

    // Header bar
    auto *headerBar = new QHBoxLayout;
    m_headerLabel = new QLabel("报警列表 — 无活跃报警", this);
    m_headerLabel->setStyleSheet("color: #aaa; font-size: 11px; font-weight: bold;");
    headerBar->addWidget(m_headerLabel);
    headerBar->addStretch();

    m_ackAllBtn = new QPushButton("确认全部", this);
    m_ackAllBtn->setStyleSheet(
        "QPushButton { background: #5a2020; border: 1px solid #8a3030; }"
        "QPushButton:hover { background: #7a3030; }");
    connect(m_ackAllBtn, &QPushButton::clicked, this, &ScreenAlarm::ackAll);
    headerBar->addWidget(m_ackAllBtn);
    mainLayout->addLayout(headerBar);

    // List area — we'll paint on this directly via paintEvent override
    // Using setMinimumSize to ensure the scroll area has space
    setMinimumHeight(300);

    mainLayout->addStretch(1);

    // Blink timer
    m_blinkTimer = new QTimer(this);
    m_blinkTimer->setInterval(500);
    connect(m_blinkTimer, &QTimer::timeout, this, [this]() {
        m_blinkPhase++;
        update();
    });
    m_blinkTimer->start();
}

void ScreenAlarm::onEnter()
{
    update();
}

void ScreenAlarm::onTick()
{
    if (!simData()) return;

    // Threshold definitions (matching stage9)
    checkTag("TEMP",  simData()->value("TEMP"),  {90, 80, 10, 5});
    checkTag("PRESS", simData()->value("PRESS"), {3.5, 2.8, 1e9, 1e9});
    checkTag("TANK",  simData()->value("TANK"),  {1e9, 1e9, 15, 5});
    checkTag("PUMP",  simData()->value("PUMP"),  {95, 90, 1e9, 1e9});
    checkTag("FLOW",  simData()->value("FLOW"),  {1e9, 1e9, 5, 1e9});

    // Update header
    m_headerLabel->setText(QString("报警列表 — 活跃 %1 | 未确认 %2")
                           .arg(m_activeCount).arg(m_unackedCount));

    update();
}

void ScreenAlarm::checkTag(const QString &tag, double value,
                           const Threshold &thresh)
{
    auto &active = m_activeKeys[tag];

    auto testHigh = [&](const QString &type, double limit) {
        if (limit > 1e8) return;
        bool isActive = active.contains(type);
        if (!isActive && value > limit) {
            active.append(type);
            fireAlarm(tag, type, limit, value);
        } else if (isActive && value <= limit * 0.98) {
            active.removeAll(type);
            // mark alarm as inactive
            for (auto &a : m_alarms) {
                if (a.tagName == tag && a.typeStr == type && a.active) {
                    a.active = false;
                    m_activeCount--;
                }
            }
        }
    };

    auto testLow = [&](const QString &type, double limit) {
        if (limit > 1e8) return;
        bool isActive = active.contains(type);
        if (!isActive && value < limit) {
            active.append(type);
            fireAlarm(tag, type, limit, value);
        } else if (isActive && value >= limit * 1.02) {
            active.removeAll(type);
            for (auto &a : m_alarms) {
                if (a.tagName == tag && a.typeStr == type && a.active) {
                    a.active = false;
                    m_activeCount--;
                }
            }
        }
    };

    testHigh("HH", thresh.hh);
    testHigh("H",  thresh.h);
    testLow ("L",  thresh.l);
    testLow ("LL", thresh.ll);
}

void ScreenAlarm::fireAlarm(const QString &tag, const QString &type,
                             double limit, double value)
{
    AlarmEntry a;
    a.id          = m_nextId++;
    a.tagName     = tag;
    a.typeStr     = type;
    a.limit       = limit;
    a.actualValue = value;
    a.timestamp   = QDateTime::currentDateTime();
    a.acknowledged = false;
    a.active      = true;

    m_alarms.prepend(a);
    m_activeCount++;
    m_unackedCount++;

    // Cap at 100
    while (m_alarms.size() > 100)
        m_alarms.removeLast();
}

void ScreenAlarm::ackAll()
{
    for (auto &a : m_alarms) {
        if (!a.acknowledged) {
            a.acknowledged = true;
            m_unackedCount--;
        }
    }
    if (m_unackedCount < 0) m_unackedCount = 0;
    update();
}

void ScreenAlarm::paintEvent(QPaintEvent *)
{
    QWidget::paintEvent(nullptr);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int w = width();
    int h = height();
    int listY = 40; // below header
    int listH = h - listY - 10;

    // List background
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(26, 29, 35));
    p.drawRoundedRect(8, listY, w - 16, listH, 6, 6);

    if (m_alarms.isEmpty()) {
        p.setPen(QColor(100, 105, 110));
        p.setFont(QFont("Microsoft YaHei", 11));
        p.drawText(QRect(8, listY, w - 16, listH),
                   Qt::AlignCenter, "无报警记录");
        return;
    }

    // Column layout
    int colX[6];
    colX[0] = 16;           // 时间
    colX[1] = 180;          // 标签
    colX[2] = 250;          // 类型
    colX[3] = 300;          // 限值
    colX[4] = 370;          // 当前值
    colX[5] = 450;          // 状态
    int colW[6] = {150, 60, 44, 60, 80, 80};

    // Header row
    p.setPen(QColor(140, 145, 155));
    p.setFont(QFont("Microsoft YaHei", 8, QFont::Medium));
    int headerY = listY + 4;
    p.drawText(QRect(colX[0], headerY, colW[0], ROW_HEIGHT),
               Qt::AlignVCenter, "时间");
    p.drawText(QRect(colX[1], headerY, colW[1], ROW_HEIGHT),
               Qt::AlignVCenter, "标签");
    p.drawText(QRect(colX[2], headerY, colW[2], ROW_HEIGHT),
               Qt::AlignVCenter, "级别");
    p.drawText(QRect(colX[3], headerY, colW[3], ROW_HEIGHT),
               Qt::AlignVCenter, "限值");
    p.drawText(QRect(colX[4], headerY, colW[4], ROW_HEIGHT),
               Qt::AlignVCenter, "当前值");
    p.drawText(QRect(colX[5], headerY, colW[5], ROW_HEIGHT),
               Qt::AlignVCenter, "状态");

    // Visible rows
    int dataY = headerY + ROW_HEIGHT;
    int maxRows = (listH - ROW_HEIGHT - 8) / ROW_HEIGHT;
    int visibleRows = qMin(maxRows, (int)m_alarms.size());

    QFont rowFont("Microsoft YaHei", 9);
    p.setFont(rowFont);

    for (int i = 0; i < visibleRows; ++i) {
        const auto &a = m_alarms[i];
        int rowY = dataY + i * ROW_HEIGHT;

        // Row background
        bool blinkOn = (m_blinkPhase % 2) == 0;
        QColor rowBg;
        if (!a.acknowledged && a.active && blinkOn)
            rowBg = QColor(180, 40, 40, 60);
        else if (!a.acknowledged && a.active && !blinkOn)
            rowBg = QColor(80, 40, 40, 40);
        else if (!a.acknowledged && !a.active && blinkOn)
            rowBg = QColor(160, 100, 20, 50);
        else if (!a.acknowledged && !a.active && !blinkOn)
            rowBg = QColor(80, 50, 20, 30);
        else if (!a.active)
            rowBg = QColor(60, 60, 65, 30);
        else
            rowBg = QColor(40, 42, 48);

        p.setPen(Qt::NoPen);
        p.setBrush(rowBg);
        p.drawRoundedRect(QRect(12, rowY, w - 24, ROW_HEIGHT - 2), 3, 3);

        QColor textColor = a.active ? QColor(220, 225, 230) : QColor(140, 145, 150);
        p.setPen(textColor);

        p.drawText(QRect(colX[0], rowY, colW[0], ROW_HEIGHT - 2),
                   Qt::AlignVCenter, a.timestamp.toString("hh:mm:ss"));
        p.drawText(QRect(colX[1], rowY, colW[1], ROW_HEIGHT - 2),
                   Qt::AlignVCenter, a.tagName);

        QColor typeC = (a.typeStr == "HH" || a.typeStr == "LL")
                       ? QColor(240, 60, 60) : QColor(255, 170, 30);
        p.setPen(typeC);
        p.setFont(QFont("Microsoft YaHei", 9, QFont::Bold));
        p.drawText(QRect(colX[2], rowY, colW[2], ROW_HEIGHT - 2),
                   Qt::AlignVCenter, a.typeStr);
        p.setFont(rowFont);

        p.setPen(textColor);
        p.drawText(QRect(colX[3], rowY, colW[3], ROW_HEIGHT - 2),
                   Qt::AlignVCenter, QString::number(a.limit, 'f', 1));
        p.setPen(a.active ? QColor(255, 255, 200) : textColor);
        p.drawText(QRect(colX[4], rowY, colW[4], ROW_HEIGHT - 2),
                   Qt::AlignVCenter, QString::number(a.actualValue, 'f', 1));

        // Status
        if (!a.active && a.acknowledged) {
            p.setPen(QColor(100, 180, 100));
            p.drawText(QRect(colX[5], rowY, colW[5], ROW_HEIGHT - 2),
                       Qt::AlignVCenter, "√ 已恢复");
        } else if (!a.active && !a.acknowledged) {
            p.setPen(QColor(255, 170, 30));
            p.drawText(QRect(colX[5], rowY, colW[5], ROW_HEIGHT - 2),
                       Qt::AlignVCenter, "▲ 待确认");
        } else if (a.active && a.acknowledged) {
            p.setPen(QColor(255, 200, 50));
            p.drawText(QRect(colX[5], rowY, colW[5], ROW_HEIGHT - 2),
                       Qt::AlignVCenter, "已确认");
        } else {
            p.setPen(QColor(240, 80, 80));
            p.drawText(QRect(colX[5], rowY, colW[5], ROW_HEIGHT - 2),
                       Qt::AlignVCenter, "● 未确认");
        }
    }
}
