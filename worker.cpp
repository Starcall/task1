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
        QFile file(file_path);
        if (file.open(QIODevice::ReadOnly)) {
            if (path_info.size() > 3) {
                auto cur = file.read(2);
                if (!validUTF8(static_cast<quint8>(cur[0]), static_cast<quint8>(cur[1]))) {
                    file.close();
                    continue;
                }
            }
            file.close();
            size_groups[path_info.size()].push_back(file_path);
        }

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
            auto lambda = [&]() -> QString {
                    QFile cur_file(file);
                    if (cur_file.open(QIODevice::ReadOnly)) {
                        QString s = cur_file.read(qMin(it.key(), qint64(7)));
                        cur_file.close();
                        return s;
                    }
                    //throw QString("Can`t open a file");
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
           // std::cerr << qq++ << std::endl;
        }

    }
    emit finish();
}

bool worker::validUTF8(quint8 a, quint8 b) {
    // 1-byte, must be followed by 1-byte or first of multi-byte
    if (a < 0x80) {
        return b < 0x80 || (0xc0 <= b && b < 0xf8);
    }
    // continuation byte, can be followed by nearly anything
    if (a < 0xC0) {
        return b < 0xf8;
    }
    // first of multi-byte, must be followed by continuation byte
    if (a < 0xF8) {
        return 0x80 <= b && b < 0xc0;
    }
    return false;
}

QByteArray worker::get_hash(QString const& filepath) {
    QCryptographicHash sha(QCryptographicHash::Sha256);
    QFile file(filepath);

    if (file.open(QIODevice::ReadOnly)) {


        sha.addData(file.readAll());
        file.close();
        return sha.result();
    }
   // throw QString("Can`t open a file");
}

