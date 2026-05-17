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
        "QPushButton:hover { background: #3a6ea8; }"
        "QGroupBox { color: #ccc; font-weight: bold; border: 1px solid #444; "
        "border-radius: 4px; margin-top: 8px; padding-top: 16px; "
        "background: #1e2128; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 12px; }"
        "QTableWidget { background: #1e2128; gridline-color: #333; "
        "border: 1px solid #444; }"
        "QTableWidget::item { color: #ddd; }"
        "QHeaderView::section { background: #2a2d35; color: #ccc; "
        "border: 1px solid #333; padding: 4px; }");

    TestWindow w;
    w.show();
    return app.exec();
}
