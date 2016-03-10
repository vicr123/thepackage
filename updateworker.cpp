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
    QProcess* updateCheck = new QProcess();
    updateCheck->start("package-query --nocolor -Au");
    updateCheck->waitForFinished(-1);

    QStringList listOfAurUpdates = QString(updateCheck->readAllStandardOutput()).split("\n");
    listOfAurUpdates.removeAll("");

    for (QString update : listOfAurUpdates) {
        if (update != "") {
            Package* pack = new Package();
            pack->setAur(true);
            pack->setPackageName(update.remove("aur/").split(" ").at(0));

            for (QString command : methods::installAurPackage(pacman, pack)) {
                shellScript.append(command);
            }
        }
    }

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

    stdOutput.append("\n[Updates complete]");
    emit output(stdOutput);
    emit finished(pacman->exitCode());
}

void UpdateWorker::outputAvaliable() {
    QString pacOutput = pacman->readAllStandardOutput();
    pacOutput.append(pacman->readAllStandardError());
    qDebug() << pacOutput;
    stdOutput.append(pacOutput);

    emit output(stdOutput);
}
