#include "pipe_widget.h"
#include <QPainter>
#include <QTimer>

PipeWidget::PipeWidget(Direction dir, QWidget *parent) : QWidget(parent), m_dir(dir)
{
    if (dir == Horizontal)
        setMinimumSize(40, 20);
    else
        setMinimumSize(20, 40);

    m_animTimer = new QTimer(this);
    m_animTimer->setInterval(80);
    connect(m_animTimer, &QTimer::timeout, this, [this]() {
        m_flowOffset = (m_flowOffset + 1) % 12;
        update();
    });
}

void PipeWidget::setFlowing(bool on)
{
    if (m_flowing == on) return;
    m_flowing = on;
    if (on) m_animTimer->start();
    else { m_animTimer->stop(); m_flowOffset = 0; }
    update();
}

void PipeWidget::setDirection(Direction dir)
{
    if (m_dir == dir) return;
    m_dir = dir;
    int t = minimumHeight();
    setMinimumHeight(minimumWidth());
    setMinimumWidth(t);
    update();
}

void PipeWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int w = width(), h = height();
    bool horiz = (m_dir == Horizontal);
    int pipeThick = horiz ? h / 2 : w / 2;
    int cx = w / 2, cy = h / 2;

    QColor pipeColor = m_flowing ? QColor(30, 140, 210) : QColor(90, 95, 100);

    // 管道主体
    p.setBrush(pipeColor);
    p.setPen(QPen(pipeColor.darker(140), 1));
    QRect pipeRect = horiz ? QRect(0, cy - pipeThick/2, w, pipeThick)
                           : QRect(cx - pipeThick/2, 0, pipeThick, h);
    p.drawRoundedRect(pipeRect, 3, 3);

    // 高光
    QColor highlight = pipeColor.lighter(150);
    highlight.setAlpha(80);
    p.setBrush(highlight);
    p.setPen(Qt::NoPen);
    if (horiz)
        p.drawRoundedRect(0, cy - pipeThick/2, w, pipeThick/3, 2, 2);
    else
        p.drawRoundedRect(cx - pipeThick/2, 0, pipeThick/3, h, 2, 2);

    // 流动虚线动画
    if (m_flowing) {
        QPen dashPen(Qt::white, pipeThick/4, Qt::DashLine);
        dashPen.setDashOffset(-m_flowOffset);
        p.setPen(dashPen);
        if (horiz)
            p.drawLine(0, cy, w, cy);
        else
            p.drawLine(cx, 0, cx, h);
    }
}
