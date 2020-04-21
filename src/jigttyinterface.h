#ifndef JIGTTYINTERFACE_H
#define JIGTTYINTERFACE_H

#include "jigabstractinterface.h"
#include <QObject>

#include <QSerialPort>

static QStringList defaultTtyCommands = {"open", "close", "usartXfer"};

class JigTtyInterface : public JigAbstractInterface
{
public:
    JigTtyInterface();

    int open() override;
    void close() override;
    void config() override;
    bool isOpen() override;
    void reset() override;
    int getResult() override;

    qint64 write(const QByteArray &data) override;

    static QStringList getDefaultCommands() {return defaultTtyCommands;}

private slots:
    void readData();

private:
    QSerialPort *sPort;

    bool debug;
    int vid;
    int pid;
    bool isVidPid;
};

#endif // JigTtyInterface_H
