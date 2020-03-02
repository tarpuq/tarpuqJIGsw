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

    QString getIpePath() const;
    void setIpePath(const QString &value);

    QString getTarget() const;
    void setTarget(const QString &value);

    QString getBur() const;
    void setBur(const QString &value);

    QString getMclrPinName() const;
    void setMclrPinName(const QString &value);

    QString getVccPinName() const;
    void setVccPinName(const QString &value);

    QString getGndPinName() const;
    void setGndPinName(const QString &value);

    QString getPgcPinName() const;
    void setPgcPinName(const QString &value);

    QString getPgdPinName() const;
    void setPgdPinName(const QString &value);

private:
    Pickit *ipe;
};

#endif // JIGINTERFACEPICKIT_H
