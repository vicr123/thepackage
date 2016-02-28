#include "updateworker.h"

UpdateWorker::UpdateWorker()
{

}

UpdateWorker::~UpdateWorker() {

}

void UpdateWorker::process() {
    pacman = new QProcess(this);
    //pacman->setProcessChannelMode(QProcess::MergedChannels);
    connect(pacman, SIGNAL(readyReadStandardOutput()), this, SLOT(outputAvaliable()));
    connect(pacman, SIGNAL(readyReadStandardError()), this, SLOT(outputAvaliable()));

    QString shellScript;

    if (QFile("/var/lib/pacman/db.lck").exists()) {
        shellScript.append("rm /var/lib/pacman/db.lck\n");
    }

    shellScript.append("pacman -Syu --noconfirm --noprogressbar --color never\n");

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
    if (pacman->exitCode() != 0) {
        QString pacOutput = pacman->readAllStandardOutput();
        pacOutput.append(pacman->readAllStandardError());
        qDebug() << pacOutput;
        stdOutput.append(pacOutput);
        stdOutput.append("***NON-ZERO EXIT CODE***");

        emit output(stdOutput);

    }

    emit finished(pacman->exitCode());
}

void UpdateWorker::outputAvaliable() {
    QString pacOutput = pacman->readAllStandardOutput();
    pacOutput.append(pacman->readAllStandardError());
    qDebug() << pacOutput;
    stdOutput.append(pacOutput);

    emit output(stdOutput);
}
