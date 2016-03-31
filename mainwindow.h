#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>
#include <QProcess>
#include <QList>
#include <QDebug>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QResizeEvent>
#include <QPropertyAnimation>
#include <QThread>
#include <QScrollBar>
#include <QFileSystemWatcher>
#include <QPointer>

#include "package.h"
#include "worker.h"
#include "updatewindow.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QList<Package*>* allPackages;
    QList<Package*>* displayedPackages;
    QList<Package*>* packagesToInstall;
    QList<Package*>* packagesToRemove;

    QList<Package*>* aurPackages;
    QList<Package*>* aurPackagesToInstall;

private slots:
    void on_lineEdit_textEdited(const QString &arg1);

    void on_pushButton_clicked();

    void on_repoTable_cellDoubleClicked(int row, int column);

    void on_pushButton_4_clicked();

    void on_repoTable_itemDoubleClicked(QTableWidgetItem *item);

    void on_repoTable_cellChanged(int row, int column);

    void on_repoTable_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

    void on_pushButton_2_clicked();

    void on_pushButton_6_clicked();

    void confirmAnimationFinished();

    void on_pushButton_5_clicked();

    void outputAvaliable(QString output);

    void commitFinished(int code);

    void on_pushButton_3_clicked();

    void on_pushButton_7_clicked();

    void on_removePacmanLock_toggled(bool checked);

    void lockFileChanged();

    void on_aurCheck_toggled(bool checked);

    void on_lineEdit_textChanged(const QString &arg1);

private:
    Ui::MainWindow *ui;

    void resizeEvent(QResizeEvent *event);

    void processTransaction(QStringList install, QStringList remove);

    bool readyToCommit = false;
    bool committing = false;
    bool stopQueryingPackageNow = false;

    QFileSystemWatcher* watcher;

    QProcess* packageQuery;
};

#endif // MAINWINDOW_H
