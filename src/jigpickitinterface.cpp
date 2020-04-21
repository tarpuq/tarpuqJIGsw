#include "jigpickitinterface.h"

JigPickitInterface::JigPickitInterface()
{
    ipe = new Pickit(this);
}

int JigPickitInterface::open()
{
    return true;
}

void JigPickitInterface::close()
{
    ipe->close();
}

void JigPickitInterface::config() {

}

void JigPickitInterface::reset()
{

}

bool JigPickitInterface::isOpen()
{
    return false;
}

int JigPickitInterface::getResult()
{
    return  0;
}

qint64 JigPickitInterface::write(const QByteArray &data)
{
    (void) data;
    return  0;
}

QString JigPickitInterface::getIpePath() const
{
    return this->parameters["ipePath"].toString();
}

void JigPickitInterface::setIpePath(const QString &value)
{
    this->parameters["ipePath"] = value;
}

QString JigPickitInterface::getTarget() const
{
    return this->parameters["target"].toString();
}

void JigPickitInterface::setTarget(const QString &value)
{
    this->parameters["target"] = value;
}

QString JigPickitInterface::getBur() const
{
    return this->parameters["bur"].toString();
}

void JigPickitInterface::setBur(const QString &value)
{
    this->parameters["bur"] = value;
}

QString JigPickitInterface::getMclrPinName() const
{
    return this->parameters["mclrPin"].toString();
}

void JigPickitInterface::setMclrPinName(const QString &value)
{
    this->parameters["mclrPin"] = value;
}

QString JigPickitInterface::getVccPinName() const
{
    return this->parameters["vccPin"].toString();
}

void JigPickitInterface::setVccPinName(const QString &value)
{
    this->parameters["vccPin"] = value;
}

QString JigPickitInterface::getGndPinName() const
{
    return this->parameters["gndPin"].toString();
}

void JigPickitInterface::setGndPinName(const QString &value)
{
    this->parameters["gndPin"] = value;
}

QString JigPickitInterface::getPgcPinName() const
{
    return this->parameters["pcgPin"].toString();
}

void JigPickitInterface::setPgcPinName(const QString &value)
{
    this->parameters["pgcPin"] = value;
}

QString JigPickitInterface::getPgdPinName() const
{
    return this->parameters["pgdPin"].toString();
}

void JigPickitInterface::setPgdPinName(const QString &value)
{
    this->parameters["pgdPin"] = value;
}
