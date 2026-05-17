#ifndef TREND_CHART_H
#define TREND_CHART_H

#include <QWidget>
#include <QHash>
#include <QVector>
#include <QColor>
#include <QPoint>
#include <QPair>

class TrendChart : public QWidget
{
    Q_OBJECT

public:
    explicit TrendChart(QWidget *parent = nullptr);

    void addCurve(const QString &name, const QColor &color);
    void clearCurves();

    // Real-time mode
    void appendData(const QString &curveName, qint64 timestamp, double value);

    // History mode
    void loadHistory(const QString &curveName,
                     const QVector<QPair<qint64, double>> &data);
    void setHistoryRange(qint64 fromTime, qint64 toTime);

    bool isRealtimeMode() const { return m_realtimeMode; }

public slots:
    void setRealtimeMode(bool realtime);
    void resetView();

signals:
    void timeRangeChanged(qint64 from, qint64 to);

protected:
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    struct CurveDef {
        QString name;
        QColor  color;
        bool    visible = true;
    };

    struct Sample {
        qint64 ts;
        double val;
    };

    static constexpr qint64 DEFAULT_RANGE_MS = 10 * 60 * 1000;
    static constexpr qint64 MIN_RANGE_MS     =  1 * 60 * 1000;
    static constexpr qint64 MAX_RANGE_MS     = 60 * 60 * 1000;
    static constexpr int    MAX_REALTIME_PTS = 6000;  // 10 min @ 100 ms
    static constexpr double ZOOM_FACTOR      = 1.3;

    QRect plotRect() const;

    QPointF dataToPixel(qint64 ts, double val, const QRect &r) const;
    qint64  pixelToTime(int x, const QRect &r) const;
    void    trimRealtimeData(const QString &name);
    void    recalcAutoYRange();
    qint64  niceTimeStep(qint64 rangeMs) const;
    double  niceValueStep(double range) const;
    static QString formatTime(qint64 tsMs, qint64 stepMs);

    QVector<CurveDef> m_curves;
    // realtime ring buffers
    QHash<QString, QVector<Sample>> m_rtData;
    // history static data
    QHash<QString, QVector<Sample>> m_histData;

    bool   m_realtimeMode  = true;
    qint64 m_timeMin       = 0;
    qint64 m_timeMax       = 0;
    double m_yMin          = 0;
    double m_yMax          = 100;
    bool   m_autoYRange    = true;

    // pan state
    bool   m_dragging      = false;
    QPoint m_dragStartPos;
    qint64 m_dragTimeMin   = 0;
    qint64 m_dragTimeMax   = 0;

    // Legend hover
    int    m_hoveredLegend = -1;
};

#endif
