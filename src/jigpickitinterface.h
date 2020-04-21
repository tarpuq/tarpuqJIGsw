#ifndef JIGPICKITINTERFACE_H
#define JIGPICKITINTERFACE_H

#include "jigabstractinterface.h"
#include "pickit.h"
#include <QObject>

static QStringList defaultUsbCommands = {"program"};

class JigPickitInterface : public JigAbstractInterface
{
public:
    JigPickitInterface();

    int open() override;
    void close() override;
    void config() override;
    void reset() override;
    bool isOpen() override;
    int getResult() override;

    qint64 write(const QByteArray &data) override;

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

#endif // JIGPICKITINTERFACE_H
