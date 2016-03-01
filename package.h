#ifndef PACKAGE_H
#define PACKAGE_H

#include <QObject>

class Package : public QObject
{
    Q_OBJECT
public:
    explicit Package(QObject *parent = 0);

    QString getPackageName();
    void setPackageName(QString name);
    QString getRepoName();
    void setRepoName(QString name);
    QString getVersion();
    void setVersion(QString version);
    bool isInstalled();
    void setInstalled(bool installed);
    bool fromAur();
    void setAur(bool aur);

signals:

public slots:

private:
    QString packageName;
    QString repoName;
    QString version;
    bool installed;
    bool aur = false;
};

#endif // PACKAGE_H
