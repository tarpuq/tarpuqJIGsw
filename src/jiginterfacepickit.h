#ifndef JIGINTERFACEPICKIT_H
#define JIGINTERFACEPICKIT_H

#include "jiginterface.h"
#include "pickit.h"
#include <QObject>

class JigInterfacePickit : public JigInterface
{
public:
    JigInterfacePickit();

    int open();
    void close();
    void config();

private:
    Pickit *ipe;
};

#endif // JIGINTERFACEPICKIT_H
