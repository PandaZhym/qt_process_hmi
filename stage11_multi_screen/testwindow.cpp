#include "testwindow.h"
#include "nav_bar.h"
#include "hmi_screen.h"
#include "sim_data_manager.h"
#include "screen_overview.h"
#include "screen_process.h"
#include "screen_alarm.h"
#include "screen_trend.h"
#include "screen_settings.h"
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QTimer>

TestWindow::TestWindow(QWidget *parent) : QWidget(parent)
{
    setupUi();
    setupSimulation();

    resize(1100, 720);
    setWindowTitle("Stage11 — Multi-Screen Navigation");
}

TestWindow::~TestWindow()
{
    m_globalTimer->stop();
}

void TestWindow::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Navigation bar
    m_navBar = new NavBar(this);
    mainLayout->addWidget(m_navBar);

    // Stacked widget for screens
    m_stackedWidget = new QStackedWidget(this);
    m_stackedWidget->setStyleSheet("QStackedWidget { background: #1a1c22; }");
    mainLayout->addWidget(m_stackedWidget, 1);

    connect(m_navBar, &NavBar::screenSelected, this, &TestWindow::onScreenSelected);
}

void TestWindow::setupSimulation()
{
    m_simData = new SimDataManager(this);

    m_globalTimer = new QTimer(this);
    m_globalTimer->setInterval(100);
    connect(m_globalTimer, &QTimer::timeout, this, &TestWindow::onGlobalTick);

    // Create and register screens
    addScreen(new ScreenOverview(this),  "总览", "≡");          // ≡
    addScreen(new ScreenProcess(this),   "流程", "◉");          // ◉
    addScreen(new ScreenAlarm(this),     "报警", "⚠");          // ⚠
    addScreen(new ScreenTrend(this),     "趋势", "▲");          // ▲
    addScreen(new ScreenSettings(this),  "设置", "⚙");          // ⚙

    activateScreen(0);
    m_globalTimer->start();
}

void TestWindow::addScreen(HmiScreen *screen, const QString &label,
                           const QString &icon)
{
    screen->setSimData(m_simData);
    int idx = m_stackedWidget->addWidget(screen);
    if (idx >= m_screens.size())
        m_screens.resize(idx + 1);
    m_screens[idx] = screen;
    m_navBar->addButton({label, icon});
}

void TestWindow::activateScreen(int index)
{
    if (index == m_activeIndex || index < 0 || index >= m_screens.size())
        return;

    if (m_activeIndex >= 0)
        m_screens[m_activeIndex]->onLeave();

    m_stackedWidget->setCurrentIndex(index);
    m_activeIndex = index;
    m_navBar->setActiveIndex(index);
    m_screens[index]->onEnter();
}

void TestWindow::onScreenSelected(int index)
{
    activateScreen(index);
}

void TestWindow::onGlobalTick()
{
    m_simData->tick();

    if (m_activeIndex >= 0 && m_activeIndex < m_screens.size())
        m_screens[m_activeIndex]->onTick();
}
