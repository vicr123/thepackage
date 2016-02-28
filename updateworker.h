#ifndef UPDATEWORKER_H
#define UPDATEWORKER_H

#include <QObject>
#include <QProcess>
#include <QDebug>
#include <QFile>
#include <QTemporaryFile>

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
    void finished(int);
    void error(QString err);
    void output(QString output);

private:
    QString stdOutput;

    QProcess* pacman;
};

#endif // UPDATEWORKER_H
