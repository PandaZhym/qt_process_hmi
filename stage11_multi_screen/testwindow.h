#ifndef TESTWINDOW_H
#define TESTWINDOW_H

#include <QWidget>
#include <QVector>

class NavBar;
class QStackedWidget;
class SimDataManager;
class HmiScreen;
class QTimer;

class TestWindow : public QWidget
{
    Q_OBJECT
public:
    explicit TestWindow(QWidget *parent = nullptr);
    ~TestWindow() override;

    void addScreen(HmiScreen *screen, const QString &label,
                   const QString &icon = QString());

private slots:
    void onGlobalTick();
    void onScreenSelected(int index);

private:
    void setupUi();
    void setupSimulation();
    void activateScreen(int index);

    NavBar          *m_navBar        = nullptr;
    QStackedWidget  *m_stackedWidget = nullptr;
    SimDataManager  *m_simData       = nullptr;
    QTimer          *m_globalTimer   = nullptr;

    QVector<HmiScreen *> m_screens;
    int m_activeIndex = -1;
};

#endif
