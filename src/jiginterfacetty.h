#ifndef JIGINTERFACETTY_H
#define JIGINTERFACETTY_H

#include "jiginterface.h"
#include <QObject>

#include <QSerialPort>

static QStringList defaultTtyCommands = {"open", "close", "usartXfer"};

class JigInterfaceTty : public JigInterface
{
public:
    JigInterfaceTty();

    int open();
    void close();
    void config();
    bool isOpen();
    qint64 write(const QByteArray &data);

    static QStringList getDefaultCommands() {return defaultTtyCommands;}

private slots:
    void readData();

private:
    QSerialPort *sPort;

    int vid;
    int pid;
    bool isVidPid;
};

#endif // JIGINTERFACETTY_H
