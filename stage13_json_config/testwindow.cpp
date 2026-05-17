#include "testwindow.h"
#include "nav_bar.h"
#include "screen_builder.h"
#include "sim_data_manager.h"
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QTimer>
#include <QLabel>
#include <QDebug>

TestWindow::TestWindow(const QStringList &configPaths, QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    loadConfigs(configPaths);

    if (!m_screens.isEmpty())
        activateScreen(0);

    resize(1100, 720);
    setWindowTitle("Stage13 — JSON Config-Driven HMI");
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

    m_navBar = new NavBar(this);
    mainLayout->addWidget(m_navBar);

    connect(m_navBar, &NavBar::screenSelected, this, &TestWindow::onScreenSelected);

    m_stackedWidget = new QStackedWidget(this);
    mainLayout->addWidget(m_stackedWidget, 1);

    // Status bar
    m_configLabel = new QLabel(this);
    m_configLabel->setStyleSheet(
        "color: #555; font-size: 9px; background: #14161a; padding: 2px 8px;");
    m_configLabel->setFixedHeight(22);
    mainLayout->addWidget(m_configLabel);
}

void TestWindow::loadConfigs(const QStringList &configPaths)
{
    m_simData = new SimDataManager(this);
    m_screenBuilder = new ScreenBuilder(m_simData, this);

    // Global timer
    m_globalTimer = new QTimer(this);
    m_globalTimer->setInterval(100);
    connect(m_globalTimer, &QTimer::timeout, this, &TestWindow::onGlobalTick);

    for (const auto &path : configPaths) {
        QWidget *screen = m_screenBuilder->buildFromJson(path);
        if (!screen) continue;

        ScreenEntry entry;
        entry.name       = path.section('/', -1).section('.', 0, 0);
        entry.configFile = path;
        entry.widget     = screen;

        int idx = m_stackedWidget->addWidget(screen);
        if (idx >= m_screens.size())
            m_screens.resize(idx + 1);
        m_screens[idx] = entry;

        // Extract display name from JSON config for NavBar
        QString navLabel = entry.name;
        QString navIcon  = (m_screens.size() == 1) ? "◉" : "≡";
        m_navBar->addButton({navLabel, navIcon});

        qDebug() << "TestWindow: loaded" << path << "→ screen" << idx;
    }

    m_globalTimer->start();
}

void TestWindow::activateScreen(int index)
{
    if (index == m_activeIndex || index < 0 || index >= m_screens.size())
        return;

    m_stackedWidget->setCurrentIndex(index);
    m_activeIndex = index;
    m_navBar->setActiveIndex(index);
    m_configLabel->setText(QString("  active config: %1").arg(m_screens[index].configFile));
}

void TestWindow::onScreenSelected(int index)
{
    activateScreen(index);
}

void TestWindow::onGlobalTick()
{
    m_simData->tick();
}
