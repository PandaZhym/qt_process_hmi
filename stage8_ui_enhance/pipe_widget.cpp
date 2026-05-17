#include "pipe_widget.h"
#include "widget_render_helpers.h"
#include <QPainter>
#include <QtMath>

PipeWidget::PipeWidget(Direction dir, QWidget *parent) : QWidget(parent), m_dir(dir)
{
    if (dir == Horizontal)
        setMinimumSize(40, 20);
    else
        setMinimumSize(20, 40);

    for (int i = 0; i < NUM_BLOBS; ++i)
        m_blobOffsets[i] = i * 1.0 / NUM_BLOBS;

    m_animTimer = new QTimer(this);
    m_animTimer->setInterval(60);
    connect(m_animTimer, &QTimer::timeout, this, [this]() {
        for (int i = 0; i < NUM_BLOBS; ++i) {
            m_blobOffsets[i] += 0.012;
            if (m_blobOffsets[i] > 1.1) m_blobOffsets[i] -= 1.1;
        }
        update();
    });
}

void PipeWidget::setFlowing(bool on)
{
    if (m_flowing == on) return;
    m_flowing = on;
    if (on) m_animTimer->start();
    else m_animTimer->stop();
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

    // ---- 1. 法兰端 ----
    p.setPen(Qt::NoPen);
    int flangeLen = horiz ? 6 : 6;
    if (horiz) {
        QLinearGradient fGrad(0, cy - pipeThick * 0.9, 0, cy + pipeThick * 0.9);
        fGrad.setColorAt(0.0, QColor(100, 105, 112));
        fGrad.setColorAt(0.5, QColor(140, 146, 154));
        fGrad.setColorAt(1.0, QColor(80, 85, 90));
        p.setBrush(fGrad);
        p.drawRoundedRect(0, cy - pipeThick * 0.9, flangeLen, pipeThick * 1.8, 2, 2);
        p.drawRoundedRect(w - flangeLen, cy - pipeThick * 0.9, flangeLen, pipeThick * 1.8, 2, 2);
    } else {
        QLinearGradient fGrad(cx - pipeThick * 0.9, 0, cx + pipeThick * 0.9, 0);
        fGrad.setColorAt(0.0, QColor(100, 105, 112));
        fGrad.setColorAt(0.5, QColor(140, 146, 154));
        fGrad.setColorAt(1.0, QColor(80, 85, 90));
        p.setBrush(fGrad);
        p.drawRoundedRect(cx - pipeThick * 0.9, 0, pipeThick * 1.8, flangeLen, 2, 2);
        p.drawRoundedRect(cx - pipeThick * 0.9, h - flangeLen, pipeThick * 1.8, flangeLen, 2, 2);
    }

    // ---- 2. 3D 管体 ----
    if (horiz) {
        int pipeLeft = flangeLen;
        int pipeRight = w - flangeLen;
        QLinearGradient bodyGrad(0, cy - pipeThick / 2.0, 0, cy + pipeThick / 2.0);
        bodyGrad.setColorAt(0.0, pipeColor.darker(150));
        bodyGrad.setColorAt(0.2, pipeColor.darker(110));
        bodyGrad.setColorAt(0.5, pipeColor.lighter(120));
        bodyGrad.setColorAt(0.8, pipeColor);
        bodyGrad.setColorAt(1.0, pipeColor.darker(140));
        p.setBrush(bodyGrad);
        p.setPen(QPen(pipeColor.darker(160), 1));
        p.drawRoundedRect(pipeLeft, cy - pipeThick / 2.0,
                          pipeRight - pipeLeft, pipeThick, 3, 3);
    } else {
        int pipeTop = flangeLen;
        int pipeBottom = h - flangeLen;
        QLinearGradient bodyGrad(cx - pipeThick / 2.0, 0, cx + pipeThick / 2.0, 0);
        bodyGrad.setColorAt(0.0, pipeColor.darker(150));
        bodyGrad.setColorAt(0.2, pipeColor.darker(110));
        bodyGrad.setColorAt(0.5, pipeColor.lighter(120));
        bodyGrad.setColorAt(0.8, pipeColor);
        bodyGrad.setColorAt(1.0, pipeColor.darker(140));
        p.setBrush(bodyGrad);
        p.setPen(QPen(pipeColor.darker(160), 1));
        p.drawRoundedRect(cx - pipeThick / 2.0, pipeTop,
                          pipeThick, pipeBottom - pipeTop, 3, 3);
    }

    // ---- 3. 粒子块流动画 ----
    if (m_flowing) {
        int blobR = pipeThick / 3;
        for (int i = 0; i < NUM_BLOBS; ++i) {
            qreal offset = m_blobOffsets[i];
            if (offset < -0.1 || offset > 1.0) continue;

            qreal alpha = 0.9;
            // 首尾渐隐
            if (offset < 0.15) alpha = offset / 0.15 * 0.9;
            else if (offset > 0.85) alpha = (1.0 - offset) / 0.15 * 0.9;

            QColor blob(180, 220, 255, (int)(alpha * 255));
            p.setBrush(blob);
            p.setPen(Qt::NoPen);

            if (horiz) {
                int bx = flangeLen + (int)((w - 2 * flangeLen) * offset);
                p.drawEllipse(QPointF(bx, cy), blobR * 1.4, blobR * 0.7);
            } else {
                int by = flangeLen + (int)((h - 2 * flangeLen) * offset);
                p.drawEllipse(QPointF(cx, by), blobR * 0.7, blobR * 1.4);
            }
        }
    }

    // ---- 4. 流向箭头（管道中点） ----
    if (m_flowing) {
        p.setPen(QPen(QColor(255, 255, 255, 160), 2));
        if (horiz) {
            int mid = w / 2;
            p.drawLine(mid - 8, cy - 5, mid, cy);
            p.drawLine(mid, cy, mid - 8, cy + 5);
            p.drawLine(mid + 2, cy - 5, mid + 10, cy);
            p.drawLine(mid + 10, cy, mid + 2, cy + 5);
        } else {
            int mid = h / 2;
            p.drawLine(cx - 5, mid - 8, cx, mid);
            p.drawLine(cx, mid, cx + 5, mid - 8);
            p.drawLine(cx - 5, mid + 2, cx, mid + 10);
            p.drawLine(cx, mid + 10, cx + 5, mid + 2);
        }
    }
}
