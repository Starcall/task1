#ifndef SCANNER_H
#define SCANNER_H

#include "QObject"

class scanner : public QObject
{
Q_OBJECT
public:
    scanner();
public slots:
    void scan_diretory(QString dir);
};

#endif // SCANNER_H
