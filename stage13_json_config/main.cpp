#include <QApplication>
#include "testwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setStyleSheet(
        "QWidget { background-color: #1a1c22; color: #ddd; "
        "font-family: 'Microsoft YaHei', 'Segoe UI', sans-serif; font-size: 12px; }"
        "QPushButton { background: #2d5a8c; border: none; border-radius: 4px; "
        "padding: 5px 14px; color: white; font-weight: bold; }"
        "QPushButton:hover { background: #3a6ea8; }");

    QStringList configs = {
        ":/configs/screen_process.json",
        ":/configs/screen_monitor.json"
    };

    TestWindow w(configs);
    w.show();
    return app.exec();
}
