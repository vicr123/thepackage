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

    if (mainWindow->aurPackagesToInstall->count() > 0) {
        for (Package *package : *mainWindow->aurPackagesToInstall) {
            //QProcess* maker = new QProcess();
            //connect(maker, SIGNAL(readyReadStandardOutput()), this, SLOT(outputAvaliable()));
            //connect(maker, SIGNAL(readyReadStandardError()), this, SLOT(outputAvaliable()));

            pacman->start("wget -O /tmp/" + package->getPackageName() + ".tar.gz https://aur.archlinux.org/cgit/aur.git/snapshot/" + package->getPackageName() + ".tar.gz");
            pacman->waitForFinished(-1);
            pacman->start("tar -xzf /tmp/" + package->getPackageName() + ".tar.gz -C /tmp/");
            pacman->waitForFinished(-1);
            pacman->setWorkingDirectory("/tmp/" + package->getPackageName());
            pacman->start("makepkg -m");
            pacman->waitForFinished(-1);
            for (QString file : QDir("/tmp/" + package->getPackageName()).entryList()) {
                if (file.endsWith(".tar.xz")) {
                    shellScript.append("pacman -U --noconfirm --noprogressbar --color never /tmp/" + package->getPackageName() + "/" + file + "\n");
                    shellScript.append("rm -rf /tmp/" + package->getPackageName() + "\n");
                    shellScript.append("rm -rf /tmp/" + package->getPackageName() + ".tar.gz" + "\n");

                }
            }
        }

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
