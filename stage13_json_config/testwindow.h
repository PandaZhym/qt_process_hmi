#ifndef TESTWINDOW_H
#define TESTWINDOW_H

#include <QWidget>
#include <QVector>

class NavBar;
class QStackedWidget;
class SimDataManager;
class ScreenBuilder;
class QTimer;
class QLabel;

class TestWindow : public QWidget
{
    Q_OBJECT
public:
    explicit TestWindow(const QStringList &configPaths,
                        QWidget *parent = nullptr);
    ~TestWindow() override;

private slots:
    void onGlobalTick();
    void onScreenSelected(int index);

private:
    void setupUi();
    void loadConfigs(const QStringList &configPaths);
    void activateScreen(int index);

    NavBar          *m_navBar         = nullptr;
    QStackedWidget  *m_stackedWidget  = nullptr;
    SimDataManager  *m_simData        = nullptr;
    ScreenBuilder   *m_screenBuilder  = nullptr;
    QTimer          *m_globalTimer    = nullptr;
    QLabel          *m_configLabel    = nullptr;

    struct ScreenEntry { QString name; QString configFile; QWidget *widget = nullptr; };
    QVector<ScreenEntry> m_screens;
    int m_activeIndex = -1;
};

#endif
