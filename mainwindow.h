#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>
#include "worker.h"

namespace Ui {
class MainWindow;
}

class main_window : public QMainWindow
{
    Q_OBJECT
    QThread* thread;

public:
    explicit main_window(QWidget *parent = 0);
    ~main_window();

private slots:
    void finish();
    void select_directory();
    void show_about_dialog();
    void show_duplicates(QVector<QString> const& data);
    void user_interrupt();
    void delete_items();
    void fill_progress_bar(qint16);
signals:
    void find_duplicates(QString const& dir);
private:
    std::unique_ptr<Ui::MainWindow> ui;
    QString directory;
    worker* thread_worker = nullptr;
    int ww = 0;

};

#endif // MAINWINDOW_H
