#include "jiginterfacepickit.h"

JigInterfacePickit::JigInterfacePickit()
{
    ipe = new Pickit(this);
}

int JigInterfacePickit::open()
{
    return true;
}

void JigInterfacePickit::close()
{
    ipe->close();
}

void JigInterfacePickit::config() {}

QString JigInterfacePickit::getIpePath() const
{
    return this->parameters["ipePath"].toString();
}

void JigInterfacePickit::setIpePath(const QString &value)
{
    this->parameters["ipePath"] = value;
}

QString JigInterfacePickit::getTarget() const
{
    return this->parameters["target"].toString();
}

void JigInterfacePickit::setTarget(const QString &value)
{
    this->parameters["target"] = value;
}

QString JigInterfacePickit::getBur() const
{
    return this->parameters["bur"].toString();
}

void JigInterfacePickit::setBur(const QString &value)
{
    this->parameters["bur"] = value;
}

QString JigInterfacePickit::getMclrPinName() const
{
    return this->parameters["mclrPin"].toString();
}

void JigInterfacePickit::setMclrPinName(const QString &value)
{
    this->parameters["mclrPin"] = value;
}

QString JigInterfacePickit::getVccPinName() const
{
    return this->parameters["vccPin"].toString();
}

void JigInterfacePickit::setVccPinName(const QString &value)
{
    this->parameters["vccPin"] = value;
}

QString JigInterfacePickit::getGndPinName() const
{
    return this->parameters["gndPin"].toString();
}

void JigInterfacePickit::setGndPinName(const QString &value)
{
    this->parameters["gndPin"] = value;
}

QString JigInterfacePickit::getPgcPinName() const
{
    return this->parameters["pcgPin"].toString();
}

void JigInterfacePickit::setPgcPinName(const QString &value)
{
    this->parameters["pgcPin"] = value;
}

QString JigInterfacePickit::getPgdPinName() const
{
    return this->parameters["pgdPin"].toString();
}

void JigInterfacePickit::setPgdPinName(const QString &value)
{
    this->parameters["pgdPin"] = value;
}
