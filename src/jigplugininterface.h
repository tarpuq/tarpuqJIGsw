#ifndef JIGPLUGININTERFACE_H
#define JIGPLUGININTERFACE_H

#include "jigabstractinterface.h"
#include <QObject>
#include <QProcess>

static QStringList defaultPluginCommands = {"open", "close", "run"};

class JigPluginInterface : public JigAbstractInterface
{
public:
    JigPluginInterface();

    enum pluginState {
        Off,
        Started,
        Error,
        Ok,
        NoOk,
    };
    Q_ENUM(pluginState)

    int open() override;
    void close() override;
    void reset() override;
    void config() override;
    bool isOpen() override;
    int getResult() override;

    qint64 write(const QByteArray &data) override;

    static QStringList getDefaultCommands() {return defaultPluginCommands;}

private slots:
    void processStdout(void);
    void processStderror(void);

private:
    QProcess *plugin;
    pluginState prvState;
    QString programPath;

};

#endif // JIGPLUGININTERFACE_H
