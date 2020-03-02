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

    int open() override;
    void close() override;
    void config() override;
    bool isOpen() override;
    qint64 write(const QByteArray &data) override;
    void reset() override;

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
