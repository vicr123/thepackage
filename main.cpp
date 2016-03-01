#include "mainwindow.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QStringList args = a.arguments();
    args.removeAt(0);

    bool showWindow = true;
    for (QString arg : args) {
        if (arg == "--updates" || arg == "-u") {
            showWindow = false;
        } else if (arg == "--help" || arg == "-h") {
            qDebug() << "thePackage";
            qDebug() << "Usage: thepackage [OPTIONS]";
            qDebug() << "  -u, --updates                Show the Updates UI instead of the Packages UI";
            qDebug() << "  -h, --help                   Show this help message";
            return 0;
        } else {
            qDebug() << arg + " is an invalid option.";
            qDebug() << "Use -h or --help to get help options.";
            return 0;
        }
    }

    MainWindow w;
    if (showWindow) {
        w.show();
    }

    return a.exec();
}
