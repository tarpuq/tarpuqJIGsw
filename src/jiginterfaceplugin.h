#ifndef JIGINTERFACEPLUGIN_H
#define JIGINTERFACEPLUGIN_H

#include "jiginterface.h"
#include <QObject>
#include <QProcess>

static QStringList defaultPluginCommands = {"open", "close", "run"};

class JigInterfacePlugin : public JigInterface
{
public:
    JigInterfacePlugin();

    enum pluginState {
        Off,
        Started,
        Error,
        Ok,
        NoOk,
    };
    Q_ENUM(pluginState)

    int open();
    void close();
    void config();
    bool isDone();
    int getResult();
    QStringList getAnswers();

    static QStringList getDefaultCommands() {return defaultPluginCommands;}

private slots:
    void processStdout(void);
    void processStderror(void);

private:
    QProcess *plugin;
    pluginState prvState;
    bool pluginDone;
    QString programPath;

};

#endif // JIGINTERFACEPLUGIN_H
