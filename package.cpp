#include "package.h"

Package::Package(QObject *parent) : QObject(parent)
{

}

QString Package::getPackageName() {
    return packageName;
}

void Package::setPackageName(QString name) {
    packageName = name;
}

QString Package::getRepoName() {
    return repoName;
}

void Package::setRepoName(QString name) {
    repoName = name;
}

QString Package::getVersion() {
    return version;
}

void Package::setVersion(QString version) {
    this->version = version;
}

bool Package::isInstalled() {
    return installed;
}

void Package::setInstalled(bool installed) {
    this->installed = installed;
}
