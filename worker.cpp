#include "worker.h"

Worker::Worker()
{

}

Worker::~Worker() {

}

void Worker::process() {
    pacman = new QProcess(this);
    //pacman->setProcessChannelMode(QProcess::MergedChannels);
    connect(pacman, SIGNAL(readyReadStandardOutput()), this, SLOT(outputAvaliable()));
    connect(pacman, SIGNAL(readyReadStandardError()), this, SLOT(outputAvaliable()));

    QString command("kdesu --noignorebutton -t bash -c ");
    QString shellScript;

    if (mainWindow->packagesToRemove->count() > 0) {
        QString toRemove;
        for (Package *package : *mainWindow->packagesToRemove) {
            toRemove.append(" " + package->getPackageName());
        }

        shellScript.append("pacman -Rs --noconfirm --noprogressbar --color never" + toRemove + "\n");
    }

    if (mainWindow->packagesToInstall->count() > 0) {
        QString toInstall;
        for (Package *package : *mainWindow->packagesToInstall) {
            toInstall.append(" " + package->getPackageName());
        }

        shellScript.append("pacman -S --noconfirm --noprogressbar --color never" + toInstall +  "\n");
    }

    QTemporaryFile* shell = new QTemporaryFile();
    shell->open();
    shell->write(shellScript.toUtf8());
    shell->flush();
    shell->close();
    QString tempFile = shell->fileName();
    shell->setPermissions(QFile::ExeOwner);
    shell->setAutoRemove(false);
    delete shell;

    command.append("");

    pacman->start("kdesu --noignorebutton -t -d \"" + tempFile + "\"");

    if (!pacman->waitForStarted(-1)) {
        emit output("Error Error :(");
        qDebug() << "Error Error!";
    }
    pacman->waitForFinished(-1);

    QFile(tempFile).remove();


    emit finished(pacman->exitCode());
}

void Worker::outputAvaliable() {
    QString pacOutput = pacman->readAllStandardOutput();
    pacOutput.append(pacman->readAllStandardError());
    qDebug() << pacOutput;
    stdOutput.append(pacOutput);

    emit output(stdOutput);
}
