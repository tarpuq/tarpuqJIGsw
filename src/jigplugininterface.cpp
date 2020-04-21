#include "jigplugininterface.h"

#include <QDebug>
#include <QFile>
#include <QtGlobal>

JigPluginInterface::JigPluginInterface()
{
    plugin = new QProcess(this);

    connect(plugin, &QProcess::readyReadStandardOutput, this, &JigPluginInterface::processStdout);
    connect(plugin, &QProcess::readyReadStandardError, this, &JigPluginInterface::processStderror);
    connect(plugin, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [this](int exitCode, QProcess::ExitStatus exitStatus) {
        (void) exitCode;
        (void) exitStatus;
        qDebug() << "Plugin End";
        this->done = true;
        this->prvState = Off;
    });
    connect(plugin, &QProcess::started, [this]() {
        qDebug() << "Plugin Start";
        this->answers.clear();
        this->done = false;
        this->prvState = Started;
    });

    this->prvState = Off;
}

int JigPluginInterface::open()
{
    int ans;

    //  Validate path
    if (!QFile(this->programPath).exists()){
        qDebug() << "Plugin path doesn't exist" << plugin->errorString();
        this->prvState = Error;
        ans = 1;
        return ans;
    }

    plugin->setArguments(this->arguments);
    plugin->setProgram(this->programPath);
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

void JigPluginInterface::close()
{
    plugin->close();
}

void JigPluginInterface::reset()
{

}

void JigPluginInterface::config()
{
    this->programPath = parameters["path"].toString();
}

bool JigPluginInterface::isOpen()
{
    return false;
}

int JigPluginInterface::getResult()
{
    return plugin->exitCode();
}

qint64 JigPluginInterface::write(const QByteArray &data)
{
    (void) data;
    return 0;
}

void JigPluginInterface::processStdout()
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

void JigPluginInterface::processStderror()
{
    QList<QByteArray> list;

    list = plugin->readAllStandardError().split('\n');

    foreach (QByteArray line, list) {
        qWarning() << line;
    }
}
