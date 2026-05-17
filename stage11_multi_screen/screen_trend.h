#ifndef SCREEN_TREND_H
#define SCREEN_TREND_H

#include "hmi_screen.h"
#include <QVector>
#include <QColor>

class ScreenTrend : public HmiScreen
{
    Q_OBJECT
public:
    explicit ScreenTrend(QWidget *parent = nullptr);

    void onEnter() override;
    void onTick() override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    struct Sample {
        qint64 ts;
        double val;
    };

    struct CurveBuf {
        QString name;
        QColor  color;
        QVector<Sample> samples;
    };

    QVector<CurveBuf> m_curves;
    static constexpr int MAX_SAMPLES = 300;
    static constexpr int MARGIN_L = 55;
    static constexpr int MARGIN_R = 20;
    static constexpr int MARGIN_T = 16;
    static constexpr int MARGIN_B = 36;
};

#endif
