#ifndef UPDATEWINDOW_H
#define UPDATEWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QThread>
#include <QScrollBar>
#include <QFileSystemWatcher>
#include <QFile>
#include <QDesktopWidget>

#include "updateworker.h"

namespace Ui {
class UpdateWindow;
}

class UpdateWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit UpdateWindow(QWidget *parent = 0);
    ~UpdateWindow();

private slots:
    void on_pushButton_2_clicked();

    void on_pushButton_clicked();

    void updateFinished(int returnCode);

    void outputAvaliable(QString output);

    void lockFileChanged();

    void on_removePacmanLock_toggled(bool checked);

private:
    Ui::UpdateWindow *ui;

    bool updatesComplete = false;
    bool committing = false;

    QFileSystemWatcher* watcher;
};

#endif // UPDATEWINDOW_H
