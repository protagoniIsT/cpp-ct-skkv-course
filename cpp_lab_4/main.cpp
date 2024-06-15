#include "MainWindow.h"

#include <QApplication>
#include <cstring>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/minesweeper_app_icon.ico"));
    bool mode = (argc > 1 && strcmp(argv[1], "dbg") == 0);
    MainWindow window(mode);
    window.setStyleSheet("background-color: rgb(153, 204, 255);");
    window.resize(400, 400);
    window.show();

    return app.exec();
}
