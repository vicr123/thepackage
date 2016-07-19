#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    packageQuery = NULL;

    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    int x = (screenGeometry.width() - this->width()) / 2;
    int y = (screenGeometry.height() - this->height()) / 2;
    this->move(x, y);

    for (QString arg : QApplication::arguments()) {
        if (arg == "--updates" || arg == "-u") {
            UpdateWindow *w = new UpdateWindow();
            w->show();
            this->close();
        }
    }

    ui->repoTable->setHorizontalHeaderLabels(QStringList() << "Package Name" << "Repository" << "Version");
    ui->repoTable->setColumnCount(3);
    ui->repoTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->progressBar->setVisible(false);
    ui->pacmanOutput->setVisible(false);
    ui->packInfoFrame->setVisible(false);
    ui->searchingBar->setVisible(false);
    ui->aurWarningFrame->setVisible(false);
    ui->label_6->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(32, 32));
    ui->label_7->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(32, 32));

    ui->commitFrame->setParent(this);
    ui->infoFrame->setParent(this);
    ui->commitFrame->setGeometry(10, -this->height(), this->width() - 20, this->height() - 20);
    ui->infoFrame->setGeometry(10, -this->height(), this->width() - 20, this->height() - 20);

    watcher = new QFileSystemWatcher(this);
    connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(lockFileChanged()));
    connect(watcher, SIGNAL(directoryChanged(QString)), this, SLOT(lockFileChanged()));
    watcher->addPath("/var/lib/pacman/db.lck");
    watcher->addPath("/var/lib/pacman/");
    lockFileChanged();

    allPackages = new QList<Package*>();
    packagesToInstall = new QList<Package*>();
    packagesToRemove = new QList<Package*>();
    displayedPackages = new QList<Package*>();
    aurPackages = new QList<Package*>();
    aurPackagesToInstall = new QList<Package*>();
    filePackagesToInstall = new QList<Package*>();

    QProcess* packsearch = new QProcess();
    packsearch->start("pacsearch -n \".*\"");
    packsearch->waitForFinished(-1);

    QString output(packsearch->readAllStandardOutput());
    qDebug() << output;
    QStringList brokenOutput = output.split("\n");

    bool processLine = true;
    for (QString package : brokenOutput) {
        if (processLine) {
            Package* pack = new Package();
            QStringList parts = package.split(" ");
            if (parts.at(0).split("/").count() != 2) {
                processLine = false;
            } else {
                ui->repoTable->setRowCount(ui->repoTable->rowCount() + 1);
                pack->setPackageName(parts.at(0).split("/").at(1));
                pack->setRepoName(parts.at(0).split("/").at(0));
                if (package.contains("[installed]")) {
                    pack->setInstalled(true);
                } else {
                    pack->setInstalled(false);
                }
                pack->setVersion(parts.at(1));

                allPackages->append(pack);
                displayedPackages->append(pack);

                int rowToAddTo = ui->repoTable->rowCount() - 1;
                QTableWidgetItem* nameItem = new QTableWidgetItem(pack->getPackageName());
                if (pack->isInstalled()) {
                    nameItem->setIcon(QIcon::fromTheme("emblem-checked"));
                } else {
                    nameItem->setIcon(QIcon::fromTheme("emblem-error"));
                }
                nameItem->setText(pack->getPackageName());
                QTableWidgetItem* repoItem = new QTableWidgetItem(pack->getRepoName());
                repoItem->setText(pack->getRepoName());
                QTableWidgetItem* versionItem = new QTableWidgetItem(pack->getVersion());
                versionItem->setText(pack->getVersion());

                ui->repoTable->setItem(rowToAddTo, 0, nameItem);
                ui->repoTable->setItem(rowToAddTo, 1, repoItem);
                ui->repoTable->setItem(rowToAddTo, 2, versionItem);
            }
        }
        processLine = !processLine;
    }

    //ui->repoTable->sortItems(0);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (committing) {
        QMessageBox::warning(this, "Committing", "We're making changes to packages. You'll need to wait for this to finish before you can close thePackage.", QMessageBox::Ok, QMessageBox::Ok);
        event->ignore();
    } else {
        event->accept();
    }
}

void MainWindow::on_lineEdit_textEdited(const QString &arg1)
{
    if (ui->aurCheck->isChecked()) {
        /*if (packageQuery != NULL) {
            //roastopQueryingPackageNow = true;
            packageQuery->terminate();

        }*/
        aurPackages->clear();
        ui->repoTable->clearContents();
        ui->repoTable->setRowCount(0);

        if (arg1.count() >= 3) {
            ui->searchingBar->setVisible(true);

            packageQuery = new QProcess();
            packageQuery->start("package-query -As --nocolor " + arg1);
            packageQuery->waitForStarted(-1);
            while (packageQuery->state() != 0) {
                QApplication::processEvents();
                if (ui->lineEdit->text() != arg1) {
                    return;
                }
            }

            if (packageQuery->exitCode() != 0) {
                return;
            }

            if (packageQuery == NULL) {
                return;
            }

            bool processLine = true;
            QString output(packageQuery->readAllStandardOutput());
            for (QString package : output.split("\n")) {
                if (!package.startsWith(" ")) {
                    Package* pack = new Package();
                    QStringList parts = package.split(" ");
                    if (parts.at(0).split("/").count() != 2) {
                        processLine = false;
                    } else {
                        ui->repoTable->setRowCount(ui->repoTable->rowCount() + 1);
                        pack->setPackageName(parts.at(0).split("/").at(1));
                        pack->setRepoName(parts.at(0).split("/").at(0));
                        if (package.contains("[installed]")) {
                            pack->setInstalled(true);
                        } else {
                            pack->setInstalled(false);
                        }
                        pack->setVersion(parts.at(1));
                        pack->setAur(true);

                        aurPackages->append(pack);

                        int rowToAddTo = ui->repoTable->rowCount() - 1;
                        QTableWidgetItem* nameItem = new QTableWidgetItem(pack->getPackageName());
                        if (pack->isInstalled()) {
                            nameItem->setIcon(QIcon::fromTheme("emblem-checked"));
                        } else {
                            nameItem->setIcon(QIcon::fromTheme("emblem-error"));
                        }
                        nameItem->setText(pack->getPackageName());
                        QTableWidgetItem* repoItem = new QTableWidgetItem(pack->getRepoName());
                        repoItem->setText(pack->getRepoName());
                        QTableWidgetItem* versionItem = new QTableWidgetItem(pack->getVersion());
                        versionItem->setText(pack->getVersion());

                        ui->repoTable->setItem(rowToAddTo, 0, nameItem);
                        ui->repoTable->setItem(rowToAddTo, 1, repoItem);
                        ui->repoTable->setItem(rowToAddTo, 2, versionItem);
                    }
                }
                processLine = !processLine;
            }
            delete packageQuery;
            packageQuery = NULL;
            ui->searchingBar->setVisible(false);
        }
    } else {
        ui->repoTable->clearContents();
        ui->repoTable->setRowCount(0);
        displayedPackages->clear();
        if (arg1 == "") {
            ui->repoTable->setRowCount(allPackages->count());
            int i = 0;
            for (Package *package : *allPackages) {
                QTableWidgetItem* nameItem = new QTableWidgetItem(package->getPackageName());
                if (package->isInstalled()) {
                    nameItem->setIcon(QIcon::fromTheme("emblem-checked"));
                } else {
                    nameItem->setIcon(QIcon::fromTheme("emblem-error"));
                }
                nameItem->setText(package->getPackageName());
                QTableWidgetItem* repoItem = new QTableWidgetItem(package->getRepoName());
                repoItem->setText(package->getRepoName());
                QTableWidgetItem* versionItem = new QTableWidgetItem(package->getVersion());
                versionItem->setText(package->getVersion());

                ui->repoTable->setItem(i, 0, nameItem);
                ui->repoTable->setItem(i, 1, repoItem);
                ui->repoTable->setItem(i, 2, versionItem);
                displayedPackages->append(package);
                i++;
            }
        } else {
            for (Package *package : *allPackages) {
                if (package->getPackageName().contains(arg1)) {
                    displayedPackages->append(package);
                    ui->repoTable->setRowCount(ui->repoTable->rowCount() + 1);

                    int rowToAddTo = ui->repoTable->rowCount() - 1;
                    QTableWidgetItem* nameItem = new QTableWidgetItem(package->getPackageName());
                    if (package->isInstalled()) {
                        nameItem->setIcon(QIcon::fromTheme("emblem-checked"));
                    } else {
                        nameItem->setIcon(QIcon::fromTheme("emblem-error"));
                    }
                    nameItem->setText(package->getPackageName());
                    QTableWidgetItem* repoItem = new QTableWidgetItem(package->getRepoName());
                    repoItem->setText(package->getRepoName());
                    QTableWidgetItem* versionItem = new QTableWidgetItem(package->getVersion());
                    versionItem->setText(package->getVersion());

                    ui->repoTable->setItem(rowToAddTo, 0, nameItem);
                    ui->repoTable->setItem(rowToAddTo, 1, repoItem);
                    ui->repoTable->setItem(rowToAddTo, 2, versionItem);
                }
            }
        }
    }
    //ui->repoTable->sortItems(0);
}

void MainWindow::on_pushButton_clicked()
{
    //QMessageBox::question(this, "debug", "This will add a package", QMessageBox::Ok, QMessageBox::Ok);

    for (QModelIndex index : ui->repoTable->selectionModel()->selectedRows()) {
        int row = index.row();
        Package* p;
        if (ui->aurCheck->isChecked()) {
            p = aurPackages->at(row);
        } else {
            p = displayedPackages->at(row);
        }

        if (!packagesToInstall->contains(p)) {

            QListWidgetItem* item = new QListWidgetItem();
            if (p->fromAur()) {
                aurPackagesToInstall->append(p);
                item->setText("(AUR) " + p->getPackageName());
            } else {
                packagesToInstall->append(p);
                item->setText(p->getPackageName());
            }

            item->setIcon(QIcon::fromTheme("list-add"));
            ui->transactionList->addItem(item);
        }
    }

    if (ui->transactionList->count() == 0) {
        ui->pushButton_2->setEnabled(false);
    } else {
        ui->pushButton_2->setEnabled(true);
    }
}

void MainWindow::on_repoTable_cellDoubleClicked(int row, int column)
{
    //if (displayedPackages->at(row)->isInstalled()) {
        //QMessageBox::question(this, "debug", "This will add " + displayedPackages->at(row)->getPackageName(), QMessageBox::Ok, QMessageBox::Ok);
        //on_pushButton_4_clicked();
    //} else {        QMessageBox::question(this, "debug", "This will remove " + displayedPackages->at(row)->getPackageName(), QMessageBox::Ok, QMessageBox::Ok);

        //on_pushButton_clicked();
    //}
}

void MainWindow::on_pushButton_4_clicked()
{
    //QMessageBox::question(this, "debug", "This will remove a package", QMessageBox::Ok, QMessageBox::Ok);

    for (QModelIndex index : ui->repoTable->selectionModel()->selectedRows()) {
        int row = index.row();
        Package* p = displayedPackages->at(row);
        p->setIncludeDeps(false);

        if (!packagesToRemove->contains(p)) {
            QListWidgetItem* item = new QListWidgetItem(p->getPackageName());
            item->setIcon(QIcon::fromTheme("edit-delete"));
            ui->transactionList->addItem(item);
            packagesToRemove->append(p);
        }
    }
    if (ui->transactionList->count() == 0) {
        ui->pushButton_2->setEnabled(false);
    } else {
        ui->pushButton_2->setEnabled(true);
    }
}

void MainWindow::on_repoTable_itemDoubleClicked(QTableWidgetItem *item)
{
}

void MainWindow::on_repoTable_cellChanged(int row, int column)
{
}

void MainWindow::on_repoTable_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    if (currentRow > -1) {
        if (ui->aurCheck->isChecked()) {
            if (aurPackages->at(currentRow)->isInstalled()) {
                ui->pushButton->setText("Reinstall Package");
                ui->pushButton_4->setText("Remove Package");
                ui->pushButton->setVisible(true);
                ui->pushButton_4->setVisible(true);
                ui->removePackageAndDependencies->setVisible(true);
            } else {
                ui->pushButton->setText("Install Package");
                ui->pushButton->setVisible(true);
                ui->pushButton_4->setVisible(false);
                ui->removePackageAndDependencies->setVisible(false);
            }
            } else {
            if (displayedPackages->at(currentRow)->isInstalled()) {
                ui->pushButton->setText("Reinstall Package");
                ui->pushButton_4->setText("Remove Package");
                ui->pushButton->setVisible(true);
                ui->pushButton_4->setVisible(true);
                ui->removePackageAndDependencies->setVisible(true);
            } else {
                ui->pushButton->setText("Install Package");
                ui->pushButton->setVisible(true);
                ui->pushButton_4->setVisible(false);
                ui->removePackageAndDependencies->setVisible(false);
            }
        }

        if (ui->aurCheck->isChecked()) {
            ui->packname->setText(ui->repoTable->item(currentRow, 0)->text());
            ui->packdep->setVisible(false);
            ui->packInfoFrame->setVisible(true);

        } else {
            QProcess* info = new QProcess(this);
            info->start("pacman -Qi " + ui->repoTable->item(currentRow, 0)->text());
            info->waitForStarted(-1);

            while (info->state() != 0) {
                QApplication::processEvents();
            }

            if (info->exitCode() != 0) {
                info->start("pacman -Si " + ui->repoTable->item(currentRow, 0)->text());
                info->waitForStarted(-1);

                while (info->state() != 0) {
                    QApplication::processEvents();
                }

            }

            QStringList packInfo = QString(info->readAllStandardOutput()).split("\n");
            packInfo.removeAll("");

            for (QString info : packInfo) {
                int index = info.length() - (info.indexOf(":") + 2);
                QString contents = info.right(index);
                if (info.startsWith("Description")) {
                    ui->packname->setText(contents);
                } else if (info.startsWith("Depends On")) {
                    ui->packdep->setText("Package Dependencies: " + contents.replace("  ", ", "));
                    ui->packdep->setVisible(true);
                }
            }

            ui->packInfoFrame->setVisible(true);
        }
    } else {
        ui->packInfoFrame->setVisible(false);
    }
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    if (ui->commitFrame->y() == 10) {
        ui->commitFrame->resize(this->width() - 20, this->height() - 20);
    }
    if (ui->infoFrame->y() == 10) {
        ui->infoFrame->resize(this->width() - 20, this->height() - 20);
    }
    //event->accept();
}

void MainWindow::on_pushButton_2_clicked()
{
    ui->confirmList->clear();
    ui->label_4->setText("Just a final check that this is what you want to do.");
    QStringList packages;
    for (Package* p : *packagesToInstall) {
        packages.append(p->getPackageName());
    }

    for (Package* p : *filePackagesToInstall) {
        packages.append(p->getPackageName());
    }

    for (Package* p : *aurPackagesToInstall) {
        packages.append(p->getPackageName() + " from AUR. Check the PKGBUILD to make sure it doesn't contain anything malicious first.");
    }

    for (Package* p : *packagesToRemove) {
        QListWidgetItem *i = new QListWidgetItem(p->getPackageName());
        i->setIcon(QIcon::fromTheme("edit-delete"));
        ui->confirmList->addItem(i);
    }

    for (QString p : packages) {
        QListWidgetItem *i = new QListWidgetItem(p);
        i->setIcon(QIcon::fromTheme("list-add"));
        ui->confirmList->addItem(i);
    }

    ui->pushButton_5->setText("Commit");
    ui->pushButton_6->setEnabled(true);
    readyToCommit = true;
    ui->pushButton_6->setVisible(true);
    ui->pacmanOutput->setVisible(false);
    ui->confirmList->setVisible(true);

    QPropertyAnimation* anim = new QPropertyAnimation(ui->commitFrame, "geometry");
    anim->setStartValue(QRect(10, this->height(), this->width() - 20, this->height() - 20));
    anim->setEndValue(QRect(10, 10, this->width() - 20, this->height() - 20));
    anim->setDuration(500);

    anim->setEasingCurve(QEasingCurve::OutCubic);

    anim->start();
}

void MainWindow::on_pushButton_6_clicked()
{
    readyToCommit = false;
    QPropertyAnimation* anim = new QPropertyAnimation(ui->commitFrame, "geometry");
    anim->setStartValue(QRect(10, 10, this->width() - 20, this->height() - 20));
    anim->setEndValue(QRect(10, this->height(), this->width() - 20, this->height() - 20));
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::OutCubic);

    connect(anim, SIGNAL(finished()), this, SLOT(confirmAnimationFinished()));

    anim->start();
}

void MainWindow::confirmAnimationFinished() {
    ui->commitFrame->setGeometry(10, -this->height(), this->width() - 20, this->height() - 20);
    ui->infoFrame->setGeometry(10, -this->height(), this->width() - 20, this->height() - 20);

}

void MainWindow::on_pushButton_5_clicked()
{
    if (readyToCommit) {
        QThread *t = new QThread();
        Worker *w = new Worker();
        w->moveToThread(t);
        w->mainWindow = this;
        //connect(w, SIGNAL(error(QString)), this, SLOT(errorString(QString)));
        connect(t, SIGNAL(started()), w, SLOT(process()));
        connect(w, SIGNAL(finished(int)), this, SLOT(commitFinished(int)));
        connect(w, SIGNAL(finished(int)), t, SLOT(quit()));
        connect(w, SIGNAL(finished(int)), w, SLOT(deleteLater()));
        connect(t, SIGNAL(finished()), t, SLOT(deleteLater()));
        connect(w, SIGNAL(output(QString)), this, SLOT(outputAvaliable(QString)));
        t->start();

        ui->pacmanOutput->setVisible(true);
        ui->confirmList->setVisible(false);
        ui->pushButton_5->setEnabled(false);
        ui->pushButton_6->setEnabled(false);
        ui->progressBar->setVisible(true);
        ui->pacmanLock->setVisible(false);
        ui->label_4->setText("Processing Transaction...");
        readyToCommit = false;
        committing = true;
    } else {
        on_pushButton_6_clicked();
    }
}

void MainWindow::outputAvaliable(QString output) {
    bool scrollToBottom = false;
    if (ui->pacmanOutput->verticalScrollBar()->value() == ui->pacmanOutput->verticalScrollBar()->maximum()) {
        scrollToBottom = true;
    }
    ui->pacmanOutput->setText(output);

    if (scrollToBottom) {
        ui->pacmanOutput->verticalScrollBar()->setValue(ui->pacmanOutput->verticalScrollBar()->maximum());
    }
}

void MainWindow::commitFinished(int code) {
    ui->pushButton_5->setText("Done");
    ui->pushButton_5->setEnabled(true);
    ui->pushButton_6->setVisible(false);
    ui->label_4->setText("The transaction completed successfully.");
    ui->progressBar->setVisible(false);
    ui->aurCheck->setChecked(false);
    readyToCommit = false;
    committing = false;

    packagesToInstall->clear();
    packagesToRemove->clear();
    ui->transactionList->clear();

    allPackages = new QList<Package*>();
    packagesToInstall = new QList<Package*>();
    packagesToRemove = new QList<Package*>();
    displayedPackages = new QList<Package*>();
    aurPackages = new QList<Package*>();
    aurPackagesToInstall = new QList<Package*>();

    QProcess* packsearch = new QProcess();
    packsearch->start("pacsearch -n \".*\"");
    packsearch->waitForFinished(-1);

    QString output(packsearch->readAllStandardOutput());
    qDebug() << output;
    QStringList brokenOutput = output.split("\n");

    bool processLine = true;
    for (QString package : brokenOutput) {
        if (processLine) {
            Package* pack = new Package();
            QStringList parts = package.split(" ");
            if (parts.at(0).split("/").count() != 2) {
                processLine = false;
            } else {
                ui->repoTable->setRowCount(ui->repoTable->rowCount() + 1);
                pack->setPackageName(parts.at(0).split("/").at(1));
                pack->setRepoName(parts.at(0).split("/").at(0));
                if (package.contains("[installed]")) {
                    pack->setInstalled(true);
                } else {
                    pack->setInstalled(false);
                }
                pack->setVersion(parts.at(1));

                allPackages->append(pack);
                displayedPackages->append(pack);

                int rowToAddTo = ui->repoTable->rowCount() - 1;
                QTableWidgetItem* nameItem = new QTableWidgetItem(pack->getPackageName());
                if (pack->isInstalled()) {
                    nameItem->setIcon(QIcon::fromTheme("emblem-checked"));
                } else {
                    nameItem->setIcon(QIcon::fromTheme("emblem-error"));
                }
                nameItem->setText(pack->getPackageName());
                QTableWidgetItem* repoItem = new QTableWidgetItem(pack->getRepoName());
                repoItem->setText(pack->getRepoName());
                QTableWidgetItem* versionItem = new QTableWidgetItem(pack->getVersion());
                versionItem->setText(pack->getVersion());

                ui->repoTable->setItem(rowToAddTo, 0, nameItem);
                ui->repoTable->setItem(rowToAddTo, 1, repoItem);
                ui->repoTable->setItem(rowToAddTo, 2, versionItem);
            }
        }
        processLine = !processLine;
    }

    ui->lineEdit->setText("");
}

void MainWindow::on_pushButton_3_clicked()
{
    if (QMessageBox::question(this, "Clear Transaction", "Do you want to clear this transaction?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes) {
        packagesToInstall->clear();
        packagesToRemove->clear();
        ui->transactionList->clear();

        ui->pushButton_2->setEnabled(false);
    }

}

void MainWindow::on_pushButton_7_clicked()
{
    UpdateWindow *w = new UpdateWindow();
    w->show();

}

void MainWindow::on_removePacmanLock_toggled(bool checked)
{
    if (checked) {
        ui->pushButton_5->setEnabled(true);
    } else {
        ui->pushButton_5->setEnabled(false);
    }
}

void MainWindow::lockFileChanged() {
    if (!committing) {
        ui->removePacmanLock->setChecked(false);

        if (QFile("/var/lib/pacman/db.lck").exists()) {
            ui->pacmanLock->setVisible(true);
            ui->pushButton_5->setEnabled(false);
        } else {
            ui->pacmanLock->setVisible(false);
            ui->pushButton_5->setEnabled(true);
        }
    }
}

void MainWindow::on_aurCheck_toggled(bool checked)
{
    ui->repoTable->clearContents();
    ui->repoTable->setRowCount(0);
    ui->aurWarningFrame->setVisible(checked);
    if (checked) {

    } else {
        QProcess* packsearch = new QProcess();
        packsearch->start("pacsearch -n \".*\"");
        packsearch->waitForFinished(-1);

        QString output(packsearch->readAllStandardOutput());
        qDebug() << output;
        QStringList brokenOutput = output.split("\n");

        bool processLine = true;
        for (QString package : brokenOutput) {
            if (processLine) {
                Package* pack = new Package();
                QStringList parts = package.split(" ");
                if (parts.at(0).split("/").count() != 2) {
                    processLine = false;
                } else {
                    ui->repoTable->setRowCount(ui->repoTable->rowCount() + 1);
                    pack->setPackageName(parts.at(0).split("/").at(1));
                    pack->setRepoName(parts.at(0).split("/").at(0));
                    if (package.contains("[installed]")) {
                        pack->setInstalled(true);
                    } else {
                        pack->setInstalled(false);
                    }
                    pack->setVersion(parts.at(1));

                    allPackages->append(pack);
                    displayedPackages->append(pack);

                    int rowToAddTo = ui->repoTable->rowCount() - 1;
                    QTableWidgetItem* nameItem = new QTableWidgetItem(pack->getPackageName());
                    if (pack->isInstalled()) {
                        nameItem->setIcon(QIcon::fromTheme("emblem-checked"));
                    } else {
                        nameItem->setIcon(QIcon::fromTheme("emblem-error"));
                    }
                    nameItem->setText(pack->getPackageName());
                    QTableWidgetItem* repoItem = new QTableWidgetItem(pack->getRepoName());
                    repoItem->setText(pack->getRepoName());
                    QTableWidgetItem* versionItem = new QTableWidgetItem(pack->getVersion());
                    versionItem->setText(pack->getVersion());

                    ui->repoTable->setItem(rowToAddTo, 0, nameItem);
                    ui->repoTable->setItem(rowToAddTo, 1, repoItem);
                    ui->repoTable->setItem(rowToAddTo, 2, versionItem);
                }
            }
            processLine = !processLine;
        }
    }
    ui->lineEdit->setText("");
}

void MainWindow::on_lineEdit_textChanged(const QString &arg1)
{

}

void MainWindow::on_actionExit_triggered()
{
    QApplication::exit();
}


void MainWindow::on_packageInfoButton_clicked()
{
    ui->infoLoadingFrame->setVisible(true);
    ui->infoDesc->setVisible(false);
    ui->infoDependencies->setVisible(false);
    ui->infoInstallReason->setVisible(false);
    ui->infoFileTree->clear();

    QPropertyAnimation* anim = new QPropertyAnimation(ui->infoFrame, "geometry");
    anim->setStartValue(QRect(10, this->height(), this->width() - 20, this->height() - 20));
    anim->setEndValue(QRect(10, 10, this->width() - 20, this->height() - 20));
    anim->setDuration(500);

    anim->setEasingCurve(QEasingCurve::OutCubic);

    anim->start();

    Package* currentPackage = displayedPackages->at(ui->repoTable->selectedItems().first()->row());
    ui->infoName->setText(currentPackage->getPackageName());

    QProcess* info = new QProcess(this);
    info->start("pacman -Qi " + currentPackage->getPackageName());
    info->waitForStarted(-1);

    while (info->state() != 0) {
        QApplication::processEvents();
    }

    QStringList packInfo = QString(info->readAllStandardOutput()).split("\n");
    packInfo.removeAll("");

    for (QString info : packInfo) {
        int index = info.length() - (info.indexOf(":") + 2);
        QString contents = info.right(index);
        if (info.startsWith("Description")) {
            ui->infoDesc->setText(contents);
            ui->infoDesc->setVisible(true);
        } else if (info.startsWith("Depends On")) {
            ui->infoDependencies->setText("Package Dependencies: " + contents.replace("  ", ", "));
            ui->infoDependencies->setVisible(true);
        } else if (info.startsWith("Install Reason")) {
            if (info.contains("Explicitly Installed")) {
                ui->infoInstallReason->setText("Install Reason: Installed by User");
            } else {
                ui->infoInstallReason->setText("Install Reason: Installed as a dependency of another package");
            }
            ui->infoInstallReason->setVisible(true);
        }
    }

    info = new QProcess(this);
    info->start("pacman -Ql " + currentPackage->getPackageName());
    info->waitForStarted(-1);

    while (info->state() != 0) {
        QApplication::processEvents();
    }

    QStringList filetree = QString(info->readAllStandardOutput()).split("\n");
    filetree.removeAll("");

    QMap<QString, QTreeWidgetItem*> map;


    for (QString file : filetree) {
        file = file.remove(0, currentPackage->getPackageName().length() + 1);
        if (file.endsWith("/")) { //This is a directory
            QStringList path = file.split("/", QString::SkipEmptyParts);
            QTreeWidgetItem* item = new QTreeWidgetItem();
            item->setText(0, path.last() + "/");

            QDir dir(file);
            dir.cdUp();

            if (dir.path() == "/") {
                ui->infoFileTree->addTopLevelItem(item);
            } else if (map.keys().contains(dir.path() + "/")) {
                map.value(dir.path() + "/")->addChild(item);
            }
            map.insert(file, item);
        } else {
            QStringList path = file.split("/");
            if (map.keys().contains(file.left(file.lastIndexOf("/") + 1))) {
                QTreeWidgetItem* item = new QTreeWidgetItem();
                item->setText(0, path.last());
                map.value(file.left(file.lastIndexOf("/") + 1))->addChild(item);
            }
        }
    }

    ui->infoLoadingFrame->setVisible(false);
}

void MainWindow::on_pushButton_8_clicked()
{
    QPropertyAnimation* anim = new QPropertyAnimation(ui->infoFrame, "geometry");
    anim->setStartValue(QRect(10, 10, this->width() - 20, this->height() - 20));
    anim->setEndValue(QRect(10, this->height(), this->width() - 20, this->height() - 20));
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::OutCubic);

    connect(anim, SIGNAL(finished()), this, SLOT(confirmAnimationFinished()));

    anim->start();
}

void MainWindow::on_removePackageAndDependencies_clicked()
{
    for (QModelIndex index : ui->repoTable->selectionModel()->selectedRows()) {
        int row = index.row();
        Package* p = displayedPackages->at(row);

        if (!packagesToRemove->contains(p)) {
            QListWidgetItem* item = new QListWidgetItem(p->getPackageName() + " + dep.");
            item->setIcon(QIcon::fromTheme("edit-delete"));
            ui->transactionList->addItem(item);
            packagesToRemove->append(p);
        }
    }
    if (ui->transactionList->count() == 0) {
        ui->pushButton_2->setEnabled(false);
    } else {
        ui->pushButton_2->setEnabled(true);
    }
}

void MainWindow::on_actionInstall_Local_Package_triggered()
{
    QFileDialog* dialog = new QFileDialog();
    dialog->setAcceptMode(QFileDialog::AcceptOpen);
    dialog->setNameFilters(QStringList() << "Packages (*.tar.xz)");
    if (dialog->exec() == QDialog::Accepted) {
        for (QString file : dialog->selectedFiles()) {
            Package* p = new Package();
            p->setInstalled(false);
            p->setPackageName(file);
            filePackagesToInstall->append(p);

            QListWidgetItem* item = new QListWidgetItem("Local File");
            item->setIcon(QIcon::fromTheme("list-add"));
            ui->transactionList->addItem(item);
        }

        if (ui->transactionList->count() == 0) {
            ui->pushButton_2->setEnabled(false);
        } else {
            ui->pushButton_2->setEnabled(true);
        }
    }
    delete dialog;
}
