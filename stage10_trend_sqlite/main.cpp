#include <QApplication>
#include "testwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setStyleSheet(
        "QWidget { background-color: #1a1c22; color: #ddd; "
        "font-family: 'Microsoft YaHei'; font-size: 12px; }");

    TestWindow w;
    w.show();

    return app.exec();
}
