#ifndef PIPE_WIDGET_H
#define PIPE_WIDGET_H

#include <QWidget>

// 管道控件 -- 显示水平或垂直管道，支持流动动画
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
    void setDirection(Direction dir);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Direction m_dir;
    bool m_flowing = false;
    int m_flowOffset = 0;
    QTimer *m_animTimer = nullptr;
};

#endif
