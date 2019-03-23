#ifndef WORKER_H
#define WORKER_H

#include <QByteArray>
#include <QVector>
#include <QString>
#include <vector>
#include <QMap>
#include <math.h>
#include <QThread>
#include <QCryptographicHash>
#include <QCommonStyle>
#include <QDesktopWidget>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QDirIterator>
#include <unordered_map>
#include <QDir>
#include <fstream>
#include <iostream>
#include <QTime>

#endif // WORKER_H

class worker : public QObject
{
    Q_OBJECT
public:
    worker(QString const& directory);
public slots:
    void run();
signals:\
    void finish();
    void send_data(QVector<QString> const& data);
    void send_progress(qint16);
private:
    QString directory;
    QByteArray get_hash(QString const& file_path);
    bool check_if_interrupted();
};
