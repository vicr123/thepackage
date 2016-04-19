#ifndef UPDATEWORKER_H
#define UPDATEWORKER_H

#include <QObject>
#include <QProcess>
#include <QDebug>
#include <QFile>
#include <QTemporaryFile>
#include "package.h"
#include "methods.h"

class UpdateWorker : public QObject
{
    Q_OBJECT
public:
    UpdateWorker();
    ~UpdateWorker();
public slots:
    void process();

    void outputAvaliable();

signals:
    void finished(bool, int);
    void error(QString err);
    void output(QString output);

private:
    QString stdOutput;

    QProcess* pacman;

    bool rebootRequired = false;
};

#endif // UPDATEWORKER_H
