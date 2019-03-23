#include "worker.h"
#include "iostream"
#include <string>

worker::worker(QString const& directory) {
    this->directory = directory;
}

void worker::run() {
    emit send_progress(1);
    //std::cerr << "lol + " << directory.toStdString() << std::endl;
    QMap<qint64, QVector<QString> > size_groups;
    QDirIterator it(directory, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        if (QThread::currentThread()->isInterruptionRequested()) {
            emit finish();
            return;
        }
        QString file_path = it.next();
        QFileInfo path_info(file_path);
        size_groups[path_info.size()].push_back(file_path);
    }
    qint16 cur = 0;
    int qq = 0;
    //std::cerr << size_groups.size() << std::endl;
    for (auto it = size_groups.begin(); it != size_groups.end(); it++) {
        cur++;
        emit send_progress(std::max(1, (cur * 100 / size_groups.size())));
        if (QThread::currentThread()->isInterruptionRequested()) {
            emit finish();
            return;
        }
        if (it.value().size() == 1) {
            continue;
        }
        QMap<QString, QVector<QString> > optimize;

        for (auto file : it.value()) {
            if (QThread::currentThread()->isInterruptionRequested()) {
                emit finish();
                return;
            }
            auto lambda = [file, it]() -> QString {
                    QFile cur_file(file);
                    if (cur_file.open(QIODevice::ReadOnly)) {
                        QString s = cur_file.read(qMin(it.key(), qint64(7)));
                        cur_file.close();
                        return s;
                    }
                    throw QString("Can`t open a file");
                };
            optimize[lambda()].push_back(file);
        }
        for (auto foo : optimize) {
            if (foo.size() == 1) continue;
            if (QThread::currentThread()->isInterruptionRequested()) {
                emit finish();
                return;
            }
            QMap<QByteArray, QVector<QString> > hash_groups;
            for (auto file : foo) {
                hash_groups[get_hash(file)].push_back(file);
            }
            for (auto it2 = hash_groups.begin(); it2 != hash_groups.end(); it2++) {
                if (QThread::currentThread()->isInterruptionRequested()) {
                    emit finish();
                    return;
                }
                if (it2.value().size() == 1) {
                    continue;
                }
               /* for (auto i : it2.value()) {
                    std::cerr << i.toStdString() << std::endl;
                }*/
                emit send_data(it2.value());
            }
            std::cerr << qq++ << std::endl;
        }

    }
    emit finish();
}

QByteArray worker::get_hash(QString const& filepath) {
    QCryptographicHash sha(QCryptographicHash::Sha256);
    QFile file(filepath);

    if (file.open(QIODevice::ReadOnly)) {
        sha.addData(file.readAll());
        file.close();
        return sha.result();
    }
    throw QString("Can`t open a file");
}

