#ifndef SCREEN_SETTINGS_H
#define SCREEN_SETTINGS_H

#include "hmi_screen.h"
#include <QTableWidget>
#include <QLabel>

class ScreenSettings : public HmiScreen
{
    Q_OBJECT
public:
    explicit ScreenSettings(QWidget *parent = nullptr);

    void onEnter() override;
    void onTick() override;

private:
    QTableWidget *m_table = nullptr;
    QLabel       *m_infoLabel = nullptr;
};

#endif
