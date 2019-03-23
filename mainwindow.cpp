#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "iostream"
#include <QCommonStyle>
#include <QDesktopWidget>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>


main_window::main_window(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), qApp->desktop()->availableGeometry()));

    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    QCommonStyle style;
    ui->actionScan_Directory->setIcon(style.standardIcon(QCommonStyle::SP_DialogOpenButton));
    ui->actionExit->setIcon(style.standardIcon(QCommonStyle::SP_DialogCloseButton));
    ui->actionAbout->setIcon(style.standardIcon(QCommonStyle::SP_DialogHelpButton));

    connect(ui->actionScan_Directory, &QAction::triggered, this, &main_window::select_directory);
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);
    connect(ui->actionAbout, &QAction::triggered, this, &main_window::show_about_dialog);
    connect(ui->stopButton, SIGNAL(released()), SLOT(user_interrupt()));
    connect(ui->deleteButton, SIGNAL(released()), SLOT(delete_items()));
    ui->stopButton->setHidden(true);
    ui->progressBar->setHidden(true);
    thread = new QThread();
}

main_window::~main_window()
{
    user_interrupt();
    thread->quit();
    thread->wait();
    delete thread;
}

void main_window::select_directory()
{
    delete thread;
    thread = new QThread();
    ui->progressBar->setHidden(false);
    QString dir = QFileDialog::getExistingDirectory(this, "Select Directory for Scanning",
                                                    QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dir.isEmpty()) return;
    directory = dir;
    std::cerr << directory.toStdString() << std::endl;
    ui->treeWidget->clear();
    ui->stopButton->setHidden(false);
    setWindowTitle(QString("Duplicates in directory - %1").arg(directory));
    thread_worker = new worker(directory);
    thread_worker->moveToThread(thread);
    qRegisterMetaType<QVector<QString>>("QVector<QString>");
   // connect(thread_worker, SIGNAL(error(QString)), this, SLOT(errorString(QString)));
    connect(thread, &QThread::finished, thread_worker, &QObject::deleteLater);
    connect(this, &main_window::find_duplicates, thread_worker, &worker::run);
    connect(thread_worker, SIGNAL(send_data(QVector<QString> const&)), this, SLOT(show_duplicates(QVector<QString> const&)));
    connect(thread_worker, &worker::finish, this, &main_window::finish);
    connect(thread_worker, SIGNAL(send_progress(qint16)), this, SLOT(fill_progress_bar(qint16)));
    thread->start();
    emit find_duplicates(directory);
}

void main_window::finish() {
    thread->quit();
    thread->wait();
    ui->stopButton->setHidden(true);
}
void main_window::show_duplicates(QVector<QString> const &data) {
    std::cerr << "asked for show\n";
    QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);
    item->setText(0, QString("There are " + QString::number(data.size())) + " files");
    QFileInfo file_info(data[0]);
    item->setText(1, QString::number(file_info.size()));
    for (QString file : data){
        QTreeWidgetItem* child_item = new QTreeWidgetItem();
        QString rel_path = "";
        for (int i = directory.size(); i < file.size(); i++)
            rel_path += file[i];
        child_item->setText(0, rel_path);
        item->addChild(child_item);
    }
    ui->treeWidget->addTopLevelItem(item);
    std::cerr << "showed " << ww++ << std::endl;
}

void main_window::show_about_dialog()
{
    QMessageBox::aboutQt(this);
}
void main_window::user_interrupt() {
    //std::cerr << "lol";
    thread->requestInterruption();
}

void main_window::fill_progress_bar(qint16 val) {
    ui->progressBar->setFormat(QString::number(val)+"%");
    ui->progressBar->setValue(val);
}
void main_window::delete_items() {
    QList<QTreeWidgetItem *> items = ui->treeWidget->selectedItems();
    if (items.empty()) {
        QMessageBox nothing;
        nothing.setText("Nothing selected");
        nothing.exec();
        return;
    }
    QMessageBox::StandardButton dialog = QMessageBox::question(this, "Deleting", "Are u sure?");
    if (dialog == QMessageBox::Yes) {
         for (auto item : items) {
            QString path = directory + '/' + item->text(0);
            QFile file(path);
            if (file.remove()) {
                if (item->parent()->childCount() == 2){
                    delete item->parent();
                } else {
                    //std::cerr << "WAT" << std::endl;
                    item->parent()->setText(0, QString("There are " + QString::number(item->parent()->childCount() - 1) + " files"));
                    item->parent()->removeChild(item);
                }
            } else {
                QMessageBox msgBox;
                msgBox.setText("Can`t delete this file for some reasons");
                msgBox.exec();
            }

        }
    }

}
