#ifndef SCREEN_PROCESS_H
#define SCREEN_PROCESS_H

#include "hmi_screen.h"

class TankWidget;
class PumpWidget;
class ValveWidget;
class PipeWidget;
class ValueDisplay;

class ScreenProcess : public HmiScreen
{
    Q_OBJECT
public:
    explicit ScreenProcess(QWidget *parent = nullptr);

    void onEnter() override;
    void onTick() override;

private:
    void setupProcessWidgets();
    void setupValueDisplays();

    TankWidget   *m_tank  = nullptr;
    PumpWidget   *m_pump  = nullptr;
    ValveWidget  *m_valve = nullptr;
    PipeWidget   *m_pipeH1 = nullptr;
    PipeWidget   *m_pipeH2 = nullptr;

    ValueDisplay *m_dispTemp  = nullptr;
    ValueDisplay *m_dispPress = nullptr;
    ValueDisplay *m_dispTank  = nullptr;
    ValueDisplay *m_dispFlow  = nullptr;
    ValueDisplay *m_dispPump  = nullptr;
    ValueDisplay *m_dispValve = nullptr;
};

#endif
