#include <QStringList>
#include <QProcess>
#include <QFile>
#include <QDir>
#include "package.h"

#ifndef METHODS_H
#define METHODS_H


class methods
{
public:
    methods();

    static QStringList installAurPackage(QProcess* pacman, Package* package);
};

#endif // METHODS_H
