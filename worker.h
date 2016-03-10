#ifndef WORKER_H
#define WORKER_H

#include <QObject>

#include "mainwindow.h"
#include "package.h"
#include "methods.h"
#include <QProcess>
#include <QTemporaryFile>
#include <QDir>

class MainWindow;

class Worker : public QObject
{
    Q_OBJECT
public:
    Worker();
    ~Worker();

    MainWindow *mainWindow;
public slots:
    void process();

    void outputAvaliable();

signals:
    void finished(int);
    void error(QString err);
    void output(QString output);

private:
    QString stdOutput;

    QProcess* pacman;

    QStringList installAurPackage(Package* package);
};

#endif // WORKER_H
