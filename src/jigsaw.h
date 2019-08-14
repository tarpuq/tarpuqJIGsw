#ifndef JIGSAW_H
#define JIGSAW_H

#include "jigcommand.h"
#include <QObject>

#include "jiginterface.h"
#include "jiginterfacepickit.h"
#include "jiginterfaceplugin.h"
#include "jiginterfacetty.h"

class JigSaw : public QObject
{
    Q_OBJECT
public:
    explicit JigSaw(QObject *parent = nullptr);

    int readProfile(QByteArray data);
    int writeProfile(QByteArray *data);

    QList<JigCommand *> commands;
    QHash<QString, JigInterface *> interfaces;

    QString getPickitMclrPinName() const;
    void setPickitMclrPinName(const QString &value);

    QString getPickitVccPinName() const;
    void setPickitVccPinName(const QString &value);

    QString getPickitGndPinName() const;
    void setPickitGndPinName(const QString &value);

    QString getPickitPgdPinName() const;
    void setPickitPgdPinName(const QString &value);

    QString getPickitPgcPinName() const;
    void setPickitPgcPinName(const QString &value);

    QString getPickitSerialNumber() const;
    void setPickitSerialNumber(const QString &value);

    QString getConfigIpePath() const;
    void setConfigIpePath(const QString &value);

    QString getConfigHexPath() const;
    void setConfigHexPath(const QString &value);

    QString getProductName() const;
    void setProductName(const QString &value);

    QString getProductCode() const;
    void setProductCode(const QString &value);

    QStringList commandList;

    QString getPickitTarget() const;
    void setPickitTarget(const QString &value);

signals:

public slots:

private:
    QString productName;
    QString productCode;

    QString pickitTarget;
    QString pickitMclrPinName;
    QString pickitVccPinName;
    QString pickitGndPinName;
    QString pickitPgdPinName;
    QString pickitPgcPinName;
    QString pickitSerialNumber;

    QString configIpePath;
    QString configHexPath;
};

#endif // JIGSAW_H
