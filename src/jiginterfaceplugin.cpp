#include "jiginterfaceplugin.h"

#include <QDebug>

JigInterfacePlugin::JigInterfacePlugin()
{
    plugin = new QProcess(this);

    connect(plugin, &QProcess::readyReadStandardOutput, this, &JigInterfacePlugin::processStdout);
    connect(plugin, &QProcess::readyReadStandardError, this, &JigInterfacePlugin::processStderror);
    connect(plugin, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), [this]() {
        qDebug() << "Plugin End";
        this->pluginDone = true;
        this->prvState = Off;
    });
    connect(plugin, &QProcess::started, [this]() {
        qDebug() << "Plugin Start";
        this->answers.clear();
        this->pluginDone = false;
        this->prvState = Started;
    });
}

int JigInterfacePlugin::open()
{
    int ans;
    plugin->setArguments(this->arguments);
    plugin->start();

    if (plugin->isOpen()) {
        qDebug() << "Open plugin OK" << plugin->program() << plugin->arguments();
        ans = 0;
    } else {
        qDebug() << "Can't open plugin" << plugin->errorString();
        ans = 1;
    }

    return ans;
}

void JigInterfacePlugin::close()
{
    plugin->close();
}

void JigInterfacePlugin::config()
{
    plugin->setProgram(parameters["path"].toString());
}

bool JigInterfacePlugin::isDone()
{
    return this->pluginDone;
}

int JigInterfacePlugin::getResult()
{
    return plugin->exitCode();
}

QStringList JigInterfacePlugin::getAnswers()
{
    return answers;
}

void JigInterfacePlugin::processStdout()
{
    QList<QByteArray> list;

    list = plugin->readAllStandardOutput().split('\n');

    foreach (QByteArray line, list) {
        if (isAnswerRequired) {
            answers << line;
        }
        qInfo() << line;
    }
}

void JigInterfacePlugin::processStderror()
{
    QList<QByteArray> list;

    list = plugin->readAllStandardError().split('\n');

    foreach (QByteArray line, list) {
        qWarning() << line;
    }
}