#include "methods.h"

methods::methods()
{

}

QStringList methods::installAurPackage(QProcess* pacman, Package* package) {
    pacman->start("wget -O /tmp/" + package->getPackageName() + ".tar.gz https://aur.archlinux.org/cgit/aur.git/snapshot/" + package->getPackageName() + ".tar.gz");
    pacman->waitForFinished(-1);
    pacman->start("tar -xzf /tmp/" + package->getPackageName() + ".tar.gz -C /tmp/");
    pacman->waitForFinished(-1);
    pacman->setWorkingDirectory("/tmp/" + package->getPackageName());

    QFile pkgbuildFile("/tmp/" + package->getPackageName() + "/PKGBUILD");
    pkgbuildFile.open(QFile::ReadOnly);
    QString pkgbuild(pkgbuildFile.readAll());
    for (QString line : pkgbuild.split("\n")) {
        if (line.startsWith("depends")) {
            QStringList dependencies = line.split("(").at(1).split(" ");
            for (QString depencency : dependencies) {
                QProcess* packageCheck = new QProcess();
                packageCheck->start("pacman -Q " + depencency.remove("'"));
                packageCheck->waitForFinished(-1);
                if (QString(packageCheck->readAllStandardOutput()) == "") {

                }
            }
        }
    }

    pacman->start("makepkg -m");
    pacman->waitForFinished(-1);

    QStringList commandsToReturn;
    for (QString file : QDir("/tmp/" + package->getPackageName()).entryList()) {
        if (file.endsWith(".tar.xz")) {
            commandsToReturn.append("pacman -U --noconfirm --noprogressbar --color never /tmp/" + package->getPackageName() + "/" + file + "\n");
            commandsToReturn.append("rm -rf /tmp/" + package->getPackageName() + "\n");
            commandsToReturn.append("rm -rf /tmp/" + package->getPackageName() + ".tar.gz" + "\n");

        }
    }

    return commandsToReturn;
}
