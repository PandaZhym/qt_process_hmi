#include "screen_builder.h"
#include "sim_data_manager.h"
#include "config_parser.h"
#include "widget_factory.h"
#include "data_binding.h"
#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>

ScreenBuilder::ScreenBuilder(SimDataManager *simData, QObject *parent)
    : QObject(parent), m_simData(simData) {}

QWidget *ScreenBuilder::buildFromJson(const QString &jsonPath)
{
    ScreenConfig config = ConfigParser::parse(jsonPath);
    if (config.widgets.isEmpty()) {
        qWarning() << "ScreenBuilder: no widgets parsed from" << jsonPath;
        return nullptr;
    }

    // Container widget
    auto *container = new QWidget;
    auto *mainLayout = new QVBoxLayout(container);
    mainLayout->setContentsMargins(12, 8, 12, 8);

    // Title bar
    auto *titleBar = new QHBoxLayout;
    auto *titleLbl = new QLabel(config.name, container);
    titleLbl->setStyleSheet("color: #aaa; font-size: 12px; font-weight: bold;");
    titleBar->addWidget(titleLbl);

    auto *srcLabel = new QLabel(QString("config: %1").arg(jsonPath), container);
    srcLabel->setStyleSheet("color: #555; font-size: 9px;");
    titleBar->addWidget(srcLabel);
    titleBar->addStretch();
    mainLayout->addLayout(titleBar);

    // Canvas
    auto *canvas = new QFrame(container);
    canvas->setStyleSheet("QFrame { background: #202328; "
                          "border: 2px solid #3a3d43; border-radius: 8px; }");
    canvas->setMinimumSize(600, 300);
    mainLayout->addWidget(canvas, 1);

    // Create each widget
    for (const auto &wd : config.widgets) {
        QWidget *w = WidgetFactory::create(wd.type, canvas);
        if (!w) continue;

        w->setGeometry(wd.x, wd.y, wd.width, wd.height);

        WidgetFactory::applyProperties(wd.type, w, wd.properties);

        DataBinding::applyInitialValues(m_simData, w, wd.type, wd.bindings);
        DataBinding::bindWidget(m_simData, w, wd.type, wd.bindings);
    }

    qDebug() << "ScreenBuilder: built screen" << config.name
             << "with" << config.widgets.size() << "widgets from" << jsonPath;

    return container;
}
