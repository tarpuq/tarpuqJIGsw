#ifndef JIGAPPINTERFACE_H
#define JIGAPPINTERFACE_H

#include "jigabstractinterface.h"
#include <QObject>

static QStringList defaultAppCommands = {"dialog","deadtime","loraserverapi","end"};

class JigAppInterface : public JigAbstractInterface
{
    Q_OBJECT
public:
    JigAppInterface();

    int open() override;
    void close() override;
    void reset() override;
    void config() override;
    bool isOpen() override;
    int getResult() override;

    qint64 write(const QByteArray &data) override;

    static QStringList getDefaultCommands() {return defaultAppCommands;}
};

#endif // JIGAPPINTERFACE_H
