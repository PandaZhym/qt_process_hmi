#ifndef PIPE_WIDGET_H
#define PIPE_WIDGET_H

#include <QWidget>
#include <QTimer>

class PipeWidget : public QWidget
{
    Q_OBJECT
public:
    enum Direction { Horizontal, Vertical };
    explicit PipeWidget(Direction dir = Horizontal, QWidget *parent = nullptr);
    Direction direction() const { return m_dir; }
    bool flowing() const { return m_flowing; }
public slots:
    void setFlowing(bool on);
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    static constexpr int NUM_BLOBS = 5;
    Direction m_dir;
    bool m_flowing = false;
    qreal m_blobOffsets[5];
    QTimer *m_animTimer = nullptr;
};
#endif
