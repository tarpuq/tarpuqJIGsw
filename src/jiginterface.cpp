#include "jiginterface.h"
#include <QDebug>

JigInterface::JigInterface(QObject *parent)
    : QObject(parent)
{
    dataOk = false;
}

void JigInterface::setTypeStr(const QString &type)
{
    if (!type.compare("app")) {
        this->type = app;
    } else if (!type.compare("usb")) {
        this->type = usb;
    } else if (!type.compare("plugin")) {
        this->type = plugin;
    } else if (!type.compare("tty")) {
        this->type = tty;
    } else {
        this->type = none;
    }
}

void JigInterface::config()
{
    switch (type) {
    case JigInterface::none:
        break;
    case JigInterface::app:
        break;
    case JigInterface::plugin:
        break;
    case JigInterface::usb:
        break;
    case JigInterface::tty:
        break;
    }
}

int JigInterface::open()
{
    return false;
}

void JigInterface::close()
{
    return;
}

bool JigInterface::isOpen()
{
    return false;
}

bool JigInterface::isDone()
{
    return false;
}

qint64 JigInterface::write(const QByteArray &data)
{
    return 0;
}

bool JigInterface::dataReady()
{
    bool ans = false;
    if (dataOk) {
        dataOk = false;
        ans = true;
    }

    return ans;
}

int JigInterface::getResult()
{
    return 1;
}

QStringList JigInterface::getAnswers()
{
    return QStringList();
}

JigInterface::JigInterfaceType JigInterface::getType() const
{
    return type;
}

QString JigInterface::getTypeStr() const
{
    QString ans;
    switch (type) {
    case JigInterface::none:
        ans = "none";
        break;
    case JigInterface::app:
        ans = "app";
        break;
    case JigInterface::plugin:
        ans = "plugin";
        break;
    case JigInterface::usb:
        ans = "usb";
        break;
    case JigInterface::tty:
        ans = "tty";
        break;
    }
    return ans;
}

void JigInterface::setType(const JigInterface::JigInterfaceType &type)
{
    this->type = type;
}

void JigInterface::setAlias(const QString &alias)
{
    this->alias = alias;
}

QString JigInterface::getAlias() const
{
    return alias;
}

QStringList JigInterface::getAnswers() const
{
    return answers;
}

void JigInterface::setAnswers(const QString &value)
{
    isAnswerRequired = true;
    expectedAnswers = value;
}

QStringList JigInterface::getArguments() const
{
    return arguments;
}

void JigInterface::setIfArguments(const QStringList &value)
{
    arguments = value;
}

void JigInterface::setIfArguments(const QString &value, const QHash<QString, QByteArray> &wildcard)
{
    QStringList list;
    QStringList::iterator it;
    bool joinArgument;
    QString aux;

    list = value.split(' ');

    joinArgument = false;
    arguments.clear();
    aux.clear();

    for (it = list.begin(); it != list.end(); it++) {
        if (it->contains('%')) {
            *it = wildcard[*it];
        }
        if (it->contains('"')) {
            it->remove('"');
            joinArgument ^= 1;
        }

        aux.append(*it);

        if (!joinArgument) {
            arguments << aux;
            aux.clear();
        } else {
            aux.append(' ');
        }
    }
}
