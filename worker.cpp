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

    /*if (mainWindow->aurPackagesToInstall->count() > 0) {
        QFile buildWait("/tmp/buildingYaourt");
        buildWait.open(QFile::write());
        buildWait.close();
        shellScript.append("while [ ! -f /tmp/buildingYaourt ]\n");
        shellScript.append("do\n");
        shellScript.append("    sleep 2\n");
        shellScript.append("done");
    }*/

    for (Package *package : *mainWindow->aurPackagesToInstall) {
        //QProcess* maker = new QProcess();
        //connect(maker, SIGNAL(readyReadStandardOutput()), this, SLOT(outputAvaliable()));
        //connect(maker, SIGNAL(readyReadStandardError()), this, SLOT(outputAvaliable()));

        for (QString command : methods::installAurPackage(pacman, package)) {
            shellScript.append(command);
        }
    }

    for (Package *package : *mainWindow->filePackagesToInstall) {
        shellScript.append("pacman -U --noconfirm --noprogressbar --color never " + package->getPackageName() + "\n");
    }

    if (mainWindow->packagesToRemove->count() > 0) {
        QString toRemove = "";
        QString toRemoveNoDeps = "";
        for (Package *package : *mainWindow->packagesToRemove) {
            if (package->includeDeps()) {
                toRemove.append(" " + package->getPackageName());
            } else {
                toRemoveNoDeps.append(" " + package->getPackageName());
            }
        }

        if (toRemove != "") {
            shellScript.append("pacman -Rs --noconfirm --noprogressbar --color never" + toRemove + "\n");
        }

        if (toRemoveNoDeps != "") {
            shellScript.append("pacman -R --noconfirm --noprogressbar --color never" + toRemoveNoDeps + "\n");
        }
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

    //pacman->start("kdesu --noignorebutton -t -d \"" + tempFile + "\"");
    pacman->start("pkexec bash \"" + tempFile + "\"");

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
