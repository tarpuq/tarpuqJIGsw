#include "jigabstractinterface.h"
#include <QDebug>

JigAbstractInterface::JigAbstractInterface(QObject *parent)
    : QObject(parent)
{
    dataOk = false;
}

void JigAbstractInterface::setTypeStr(const QString &type)
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

QString JigAbstractInterface::getTypeStr() const
{
    QString ans;
    switch (type) {
    case JigAbstractInterface::none:
        ans = "none";
        break;
    case JigAbstractInterface::app:
        ans = "app";
        break;
    case JigAbstractInterface::plugin:
        ans = "plugin";
        break;
    case JigAbstractInterface::usb:
        ans = "usb";
        break;
    case JigAbstractInterface::tty:
        ans = "tty";
        break;
    }
    return ans;
}

void JigAbstractInterface::setType(const JigAbstractInterface::JigInterfaceType &type)
{
    this->type = type;
}

JigAbstractInterface::JigInterfaceType JigAbstractInterface::getType() const
{
    return type;
}

void JigAbstractInterface::setAlias(const QString &alias)
{
    this->alias = alias;
}

QString JigAbstractInterface::getAlias() const
{
    return alias;
}

void JigAbstractInterface::setAnswers(const QString &value)
{
    isAnswerRequired = true;
    expectedAnswers = value;
}

QStringList JigAbstractInterface::getAnswers() const
{
    return answers;
}

QStringList JigAbstractInterface::getArguments() const
{
    return arguments;
}

void JigAbstractInterface::setIfArguments(const QStringList &value)
{
    arguments = value;
}

void JigAbstractInterface::setIfArguments(const QString &value, const QHash<QString, QByteArray> &wildcard)
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

bool JigAbstractInterface::isDone()
{
    return done;
}

bool JigAbstractInterface::dataReady()
{
    bool ans = dataOk;
    dataOk = false;
    return ans;
}

