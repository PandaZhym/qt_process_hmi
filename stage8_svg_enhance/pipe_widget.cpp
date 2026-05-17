#include "pipe_widget.h"
#include "svg_assets.h"
#include "svg_renderer_pool.h"
#include <QPainter>
#include <QSvgRenderer>

PipeWidget::PipeWidget(Direction dir, QWidget *parent) : QWidget(parent), m_dir(dir)
{
    setMinimumSize(dir == Horizontal ? 40 : 20, dir == Horizontal ? 20 : 40);
    for (int i = 0; i < NUM_BLOBS; ++i)
        m_blobOffsets[i] = i * 1.0 / NUM_BLOBS;

    m_animTimer = new QTimer(this);
    m_animTimer->setInterval(60);
    connect(m_animTimer, &QTimer::timeout, this, [this]() {
        for (int i = 0; i < NUM_BLOBS; ++i) {
            m_blobOffsets[i] += 0.015;
            if (m_blobOffsets[i] > 1.1) m_blobOffsets[i] -= 1.1;
        }
        update();
    });

    auto &pool = SvgRendererPool::instance();
    if (m_dir == Horizontal && !pool.renderer("pipe_h"))
        pool.add("pipe_h", QByteArray(svg_assets::PIPE_HORIZ));
    else if (m_dir == Vertical && !pool.renderer("pipe_v"))
        pool.add("pipe_v", QByteArray(svg_assets::PIPE_VERT));
}

void PipeWidget::setFlowing(bool on)
{
    if (m_flowing == on) return;
    m_flowing = on;
    if (on) m_animTimer->start();
    else m_animTimer->stop();
    update();
}

void PipeWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    bool horiz = (m_dir == Horizontal);
    int w = width(), h = height();
    int pipeThick = horiz ? h / 2 : w / 2;
    int margin = horiz ? w * 0.05 : h * 0.05;
    int cx = w / 2, cy = h / 2;

    QColor pipeColor = m_flowing ? QColor(30, 140, 210) : QColor(90, 95, 100);

    // ---- SVG 静态管体 ----
    QSvgRenderer *pipeSvg = SvgRendererPool::instance().renderer(horiz ? "pipe_h" : "pipe_v");
    if (pipeSvg) {
        QRectF svgRect(0, 0, w, h);
        pipeSvg->render(&p, svgRect);
    }

    // ---- 流动粒子覆盖 ----
    if (m_flowing) {
        int blobR = pipeThick / 3;
        int start = margin + pipeThick * 0.8;
        int end = (horiz ? w : h) - margin - pipeThick * 0.8;
        int len = end - start;

        for (int i = 0; i < NUM_BLOBS; ++i) {
            qreal offset = m_blobOffsets[i];
            if (offset < -0.1 || offset > 1.0) continue;

            qreal alpha = 0.9;
            if (offset < 0.15) alpha = offset / 0.15 * 0.9;
            else if (offset > 0.85) alpha = (1.0 - offset) / 0.15 * 0.9;

            QColor blob(180, 220, 255, (int)(alpha * 255));
            p.setBrush(blob);
            p.setPen(Qt::NoPen);

            int pos = start + (int)(len * offset);
            if (horiz)
                p.drawEllipse(QPointF(pos, cy), blobR * 1.3, blobR * 0.6);
            else
                p.drawEllipse(QPointF(cx, pos), blobR * 0.6, blobR * 1.3);
        }
    }

    // 流向箭头
    if (m_flowing) {
        p.setPen(QPen(QColor(255, 255, 255, 140), 1.5));
        if (horiz) {
            int mid = w / 2;
            p.drawLine(mid - 6, cy - 4, mid, cy);
            p.drawLine(mid, cy, mid - 6, cy + 4);
            p.drawLine(mid + 2, cy - 4, mid + 8, cy);
            p.drawLine(mid + 8, cy, mid + 2, cy + 4);
        } else {
            int mid = h / 2;
            p.drawLine(cx - 4, mid - 6, cx, mid);
            p.drawLine(cx, mid, cx + 4, mid - 6);
            p.drawLine(cx - 4, mid + 2, cx, mid + 8);
            p.drawLine(cx, mid + 8, cx + 4, mid + 2);
        }
    }
}
