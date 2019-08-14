#ifndef JIGINTERFACETTY_H
#define JIGINTERFACETTY_H

#include "jiginterface.h"
#include <QObject>

#include <QSerialPort>

class JigInterfaceTty : public JigInterface
{
public:
    JigInterfaceTty();

    int open();
    void close();
    void config();
    bool isOpen();
    qint64 write(const QByteArray &data);

private slots:
    void readData();

private:
    QSerialPort *sPort;

    int vid;
    int pid;
    bool isVidPid;
};

#endif // JIGINTERFACETTY_H
