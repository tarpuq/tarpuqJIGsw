#ifndef JIGINTERFACEPICKIT_H
#define JIGINTERFACEPICKIT_H

#include "jiginterface.h"
#include "pickit.h"
#include <QObject>

static QStringList defaultUsbCommands = {"program"};

class JigInterfacePickit : public JigInterface
{
public:
    JigInterfacePickit();

    int open();
    void close();
    void config();

    static QStringList getDefaultCommands() {return defaultUsbCommands;}

private:
    Pickit *ipe;
};

#endif // JIGINTERFACEPICKIT_H
