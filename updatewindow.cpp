#include "updatewindow.h"
#include "ui_updatewindow.h"

UpdateWindow::UpdateWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::UpdateWindow)
{
    ui->setupUi(this);

    ui->pushButton->setEnabled(false);
    ui->pushButton_2->setEnabled(false);
    ui->powerWarning->setVisible(false);
    ui->pacmanOutput->setVisible(false);
    ui->label_6->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(32, 32));

    watcher = new QFileSystemWatcher(this);
    connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(lockFileChanged()));
    connect(watcher, SIGNAL(directoryChanged(QString)), this, SLOT(lockFileChanged()));
    watcher->addPath("/var/lib/pacman/db.lck");
    watcher->addPath("/var/lib/pacman/");
    lockFileChanged();

    QProcess* updateCheck = new QProcess();
    updateCheck->start("checkupdates");
    updateCheck->waitForStarted(-1);

    this->show();

    while (updateCheck->state() != 0) {
        QApplication::processEvents();
    }

    QStringList listOfUpdates = QString(updateCheck->readAllStandardOutput()).split("\n");
    listOfUpdates.removeAll("");

    for (QString update : listOfUpdates) {
        if (update != "") {
            QListWidgetItem *i = new QListWidgetItem(update);
            i->setIcon(QIcon::fromTheme("application-x-executable"));
            ui->updatesAvaliable->addItem(i);
        }
    }

    if (!QFile("/var/lib/pacman/db.lck").exists() || ui->removePacmanLock->isChecked()) {
        ui->pushButton->setEnabled(true);
    }
    ui->pushButton_2->setEnabled(true);
    ui->progressBar->setVisible(false);

    if (listOfUpdates.count() == 0) {
        ui->label->setText("Thanks for stopping by. There aren't any new updates avaliable.");
        ui->pushButton->setEnabled(false);
    }

}

UpdateWindow::~UpdateWindow()
{
    delete ui;
}

void UpdateWindow::on_pushButton_2_clicked()
{
    this->close();
}

void UpdateWindow::on_pushButton_clicked()
{
    if (updatesComplete) {
        this->close();
    } else {
        committing = true;
        ui->pushButton->setEnabled(false);
        ui->pushButton_2->setEnabled(false);
        ui->progressBar->setVisible(true);
        ui->pacmanOutput->setVisible(true);
        ui->updatesAvaliable->setVisible(false);
        ui->pacmanLock->setVisible(false);
        ui->label->setText("Processing Updates...");

        QThread *t = new QThread();
        UpdateWorker *w = new UpdateWorker();
        w->moveToThread(t);
        //connect(w, SIGNAL(error(QString)), this, SLOT(errorString(QString)));
        connect(t, SIGNAL(started()), w, SLOT(process()));
        connect(w, SIGNAL(finished(int)), this, SLOT(updateFinished(int)));
        connect(w, SIGNAL(finished(int)), t, SLOT(quit()));
        connect(w, SIGNAL(finished(int)), w, SLOT(deleteLater()));
        connect(t, SIGNAL(finished()), t, SLOT(deleteLater()));
        connect(w, SIGNAL(output(QString)), this, SLOT(outputAvaliable(QString)));
        t->start();
    }
}

void UpdateWindow::updateFinished(int returnCode) {
    ui->label->setText("Updates complete.");
    ui->pushButton->setText("Finish");
    ui->pushButton->setIcon(QIcon::fromTheme("dialog-ok"));
    ui->pushButton->setEnabled(true);
    ui->pushButton_2->setVisible(false);
    ui->progressBar->setVisible(false);
    updatesComplete = true;
}

void UpdateWindow::outputAvaliable(QString output) {
    bool scrollToBottom = false;
    if (ui->pacmanOutput->verticalScrollBar()->value() == ui->pacmanOutput->verticalScrollBar()->maximum()) {
        scrollToBottom = true;
    }
    ui->pacmanOutput->setText(output);

    if (scrollToBottom) {
        ui->pacmanOutput->verticalScrollBar()->setValue(ui->pacmanOutput->verticalScrollBar()->maximum());
    }
}

void UpdateWindow::lockFileChanged() {
    if (!committing) {
        ui->removePacmanLock->setChecked(false);

        if (QFile("/var/lib/pacman/db.lck").exists()) {
            ui->pacmanLock->setVisible(true);
            if (ui->updatesAvaliable->count() > 0) {
                ui->pushButton->setEnabled(false);
            }
        } else {
            ui->pacmanLock->setVisible(false);
            if (ui->updatesAvaliable->count() > 0) {
                ui->pushButton->setEnabled(true);
            }
        }
    }
}

void UpdateWindow::on_removePacmanLock_toggled(bool checked)
{

    if (ui->updatesAvaliable->count() > 0) {
        if (checked) {
            ui->pushButton->setEnabled(true);
        } else {
            ui->pushButton->setEnabled(false);
        }
    }
}
