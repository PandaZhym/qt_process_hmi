#ifndef HMI_SCREEN_H
#define HMI_SCREEN_H

#include <QWidget>

class SimDataManager;

class HmiScreen : public QWidget
{
    Q_OBJECT
public:
    explicit HmiScreen(QWidget *parent = nullptr) : QWidget(parent) {}

    virtual void onEnter() {}
    virtual void onLeave() {}
    virtual void onTick() {}

    void setSimData(SimDataManager *sim) { m_simData = sim; }

protected:
    SimDataManager *simData() const { return m_simData; }

private:
    SimDataManager *m_simData = nullptr;
};

#endif
