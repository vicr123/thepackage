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
    bool includeDeps();
    void setIncludeDeps(bool deps);

signals:

public slots:

private:
    QString packageName;
    QString repoName;
    QString version;
    bool installed;
    bool aur = false;
    bool deps = true;
};

#endif // PACKAGE_H
