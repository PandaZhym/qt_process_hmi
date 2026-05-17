#ifndef TESTWINDOW_H
#define TESTWINDOW_H

#include <QWidget>
#include <QTimer>
#include "tank_widget.h"
#include "pump_widget.h"
#include "valve_widget.h"
#include "pipe_widget.h"
#include "value_display.h"

class TestWindow : public QWidget
{
    Q_OBJECT
public:
    explicit TestWindow(QWidget *parent = nullptr);
private slots:
    void tick();
private:
    void setupUi();
    void applyTheme();

    TankWidget   *m_tank   = nullptr;
    PumpWidget   *m_pump   = nullptr;
    ValveWidget  *m_valve  = nullptr;
    PipeWidget   *m_pipe1  = nullptr, *m_pipe2 = nullptr, *m_pipe3 = nullptr;
    ValueDisplay *m_tempDisp = nullptr, *m_pressDisp = nullptr, *m_flowDisp = nullptr;
    QTimer *m_simTimer = nullptr;
    qreal m_simPhase = 0;
};
#endif
