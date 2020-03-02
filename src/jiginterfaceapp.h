#ifndef JIGINTERFACEAPP_H
#define JIGINTERFACEAPP_H

#include "jiginterface.h"
#include <QObject>

static QStringList defaultAppCommands = {"dialog","deadtime","loraserverapi","end"};

class JigInterfaceApp : public JigInterface
{
    Q_OBJECT
public:
    JigInterfaceApp();

    static QStringList getDefaultCommands() {return defaultAppCommands;}

};

#endif // JIGINTERFACEAPP_H
