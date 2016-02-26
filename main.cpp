#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    bool showWindow = true;
    for (QString arg : a.arguments()) {
        if (arg == "--updates" || arg == "-u") {
            showWindow = false;
        }
    }

    MainWindow w;
    if (showWindow) {
        w.show();
    }

    return a.exec();
}
