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

    QString shellScript;

    if (QFile("/var/lib/pacman/db.lck").exists()) {
        shellScript.append("rm /var/lib/pacman/db.lck\n");
    }

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

    pacman->start("kdesu --noignorebutton -t -d \"" + tempFile + "\"");

    if (!pacman->waitForStarted(-1)) {
        emit output("Error Error :(");
        qDebug() << "Error Error!";
    }
    pacman->waitForFinished(-1);

    QFile(tempFile).remove();

    stdOutput.append("\n[Installation complete]");
    emit output(stdOutput);
    emit finished(pacman->exitCode());
}

void Worker::outputAvaliable() {
    QString pacOutput = pacman->readAllStandardOutput();
    pacOutput.append(pacman->readAllStandardError());
    pacOutput.replace("[Y/n]", "");
    qDebug() << pacOutput;
    stdOutput.append(pacOutput);

    emit output(stdOutput);
}
