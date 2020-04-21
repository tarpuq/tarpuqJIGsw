#ifndef JIGSAW_H
#define JIGSAW_H

#include "jigsynccommand.h"
#include <QObject>

#include "jigappinterface.h"
#include "jigpickitinterface.h"
#include "jigplugininterface.h"
#include "jigttyinterface.h"

class JigProfile : public QObject
{
    Q_OBJECT
public:
    explicit JigProfile(QObject *parent = nullptr);

    int readProfile(QString path);
    int writeProfile(QString path);

    QList<JigSyncCommand *> syncCommands;
    QList<JigSyncCommand *> asyncCommands;
    QHash<QString, JigAbstractInterface *> interfaces;

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

    QByteArray getTopPictureBase64() const;
    void setTopPictureBase64(const QByteArray &value);

    QByteArray getBottomPictureBase64() const;
    void setBottomPictureBase64(const QByteArray &value);

signals:

public slots:

private:
    QString productName;
    QString productCode;
    bool isPanelized;
    int panelRows;
    int panelCols;
    int panelAmount;
    QByteArray topPictureBase64;
    QByteArray bottomPictureBase64;

    QString fwPath;
};

#endif // JIGSAW_H
