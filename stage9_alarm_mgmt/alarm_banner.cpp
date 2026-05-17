#include "alarm_banner.h"
#include <QPainter>
#include <QMouseEvent>
#include <QtMath>

AlarmBanner::AlarmBanner(QWidget *parent) : QWidget(parent)
{
    setFixedHeight(36);
    setVisible(false);

    m_pulseTimer = new QTimer(this);
    m_pulseTimer->setInterval(400);
    connect(m_pulseTimer, &QTimer::timeout, this, [this]() {
        m_pulsePhase += 0.4;
        update();
    });
}

void AlarmBanner::updateState(int activeCount, int unackedCount,
                               const AlarmRecord *latestUnacked)
{
    m_activeCount  = activeCount;
    m_unackedCount = unackedCount;

    bool hasUnacked = unackedCount > 0;
    setVisible(hasUnacked);

    if (hasUnacked) {
        m_pulseTimer->start();
        if (latestUnacked)
            m_latestMsg = QString("%1 %2 报警: %3")
                .arg(latestUnacked->tagName)
                .arg(alarmTypeName(latestUnacked->type))
                .arg(latestUnacked->actualValue, 0, 'f', 1);
        else
            m_latestMsg = "无报警详情";
    } else {
        m_pulseTimer->stop();
    }
    update();
}

void AlarmBanner::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int w = width(), h = height();

    // 脉动红色背景
    qreal pulseIntensity = 0.5 + 0.3 * qSin(m_pulsePhase * M_PI);
    QColor bg1(180, 20, 20, (int)(150 + 50 * pulseIntensity));
    QColor bg2(120, 10, 10, (int)(150 + 50 * pulseIntensity));

    QLinearGradient grad(0, 0, 0, h);
    grad.setColorAt(0.0, bg1);
    grad.setColorAt(1.0, bg2);
    p.setBrush(grad);
    p.setPen(QPen(QColor(220, 40, 40), 1));
    p.drawRoundedRect(0, 0, w, h, 6, 6);

    // 图标
    p.setPen(Qt::white);
    p.setFont(QFont(font().family(), 14, QFont::Bold));
    p.drawText(QRect(12, 0, 30, h), Qt::AlignCenter, "⚠");

    // 文字（区分活跃和已恢复）
    QFont msgFont(font().family(), 11);
    p.setFont(msgFont);
    p.setPen(QColor(255, 255, 230));
    // unacked 包含已恢复待确认，active 仅统计活跃的
    int recovered = m_unackedCount - m_activeCount;
    if (recovered < 0) recovered = 0;
    QString text;
    if (m_unackedCount == 1)
        text = QString("1 条报警未确认 — %1").arg(m_latestMsg);
    else if (recovered > 0)
        text = QString("%1 条未确认（%2 条已恢复待确认） — %3")
                   .arg(m_unackedCount).arg(recovered).arg(m_latestMsg);
    else
        text = QString("%1 条报警未确认 — %2")
                   .arg(m_unackedCount).arg(m_latestMsg);
    p.drawText(QRect(46, 0, w - 100, h), Qt::AlignLeft | Qt::AlignVCenter, text);

    // 点击提示
    p.setPen(QColor(255, 255, 255, 140));
    p.setFont(QFont(font().family(), 9));
    p.drawText(QRect(w - 120, 0, 110, h), Qt::AlignRight | Qt::AlignVCenter,
               "点击确认全部");
}

void AlarmBanner::mousePressEvent(QMouseEvent *)
{
    emit clicked();
}
