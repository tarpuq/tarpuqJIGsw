#ifndef JIGSAW_H
#define JIGSAW_H

#include "jigsynccommand.h"
#include <QObject>

#include "jiginterfaceapp.h"
#include "jiginterfacepickit.h"
#include "jiginterfaceplugin.h"
#include "jiginterfacetty.h"

class JigProfile : public QObject
{
    Q_OBJECT
public:
    explicit JigProfile(QObject *parent = nullptr);

    int readProfile(QByteArray data);
    int writeProfile(QByteArray *data);

    QList<JigSyncCommand *> syncCommands;
    QList<JigSyncCommand *> asyncCommands;
    QHash<QString, JigInterface *> interfaces;

    QString getFwPath() const;
    void setFwPath(const QString &value);

    QString getProductName() const;
    void setProductName(const QString &value);

    QString getProductCode() const;
    void setProductCode(const QString &value);

    bool getIsPanelized() const;
    void setIsPanelized(bool value);

    int getPanelRows() const;
    void setPanelRows(int value);

    int getPanelCols() const;
    void setPanelCols(int value);

    int getPanelAmount() const;
    void setPanelAmount(int value);

    QStringList availableInterfaces;

    QString getPickitTarget() const;
    void setPickitTarget(const QString &value);

    bool isCommandValid(JigSyncCommand *command);


signals:

public slots:

private:
    QString productName;
    QString productCode;
    bool isPanelized;
    int panelRows;
    int panelCols;
    int panelAmount;

    QString fwPath;
};

#endif // JIGSAW_H
