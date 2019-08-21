#include "jigsaw.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <QDebug>
#include <QMessageBox>

JigSaw::JigSaw(QObject *parent)
    : QObject(parent)
{
    commandList.clear();
    commandList << "program";
    commandList << "usartXfer";
    commandList << "dialog";
    commandList << "deadtime";
    commandList << "run";
    commandList << "open";
    commandList << "close";
    commandList << "loraserverapi";
    commandList << "end";
}

int JigSaw::readProfile(QByteArray data)
{
    int retCode = 0;
    QJsonArray::iterator arrayIterator;
    QString error;

    QJsonDocument json_document(QJsonDocument::fromJson(data));

    if (json_document.isNull()) {
        qDebug() << "Error en el archivo JSON";
        retCode = -1;
        return retCode;
    }

    QJsonObject json = json_document.object();

    commands.clear();
    interfaces.clear();

    //      Process
    if (json.contains("productName") && json["productName"].isString()) {
        productName = json["productName"].toString();
    }

    if (json.contains("productCode") && json["productCode"].isString()) {
        productCode = json["productCode"].toString();
    }

    if (json.contains("interfaces") && json["interfaces"].isArray()) {
        QJsonArray interfacesArray = json["interfaces"].toArray();

        for (arrayIterator = interfacesArray.begin(); arrayIterator < interfacesArray.end();
             arrayIterator++) {
            QJsonObject interfaceObject = arrayIterator->toObject();

            if (interfaceObject.contains("alias") && interfaceObject["alias"].isString()) {
                QString type = interfaceObject["type"].toString();
                QString alias = interfaceObject["alias"].toString();

                JigInterface *iface = nullptr;

                if (type == "app") {
                    iface = new JigInterface(this);
                } else if (type == "plugin") {
                    iface = new JigInterfacePlugin();
                } else if (type == "tty") {
                    iface = new JigInterfaceTty();
                } else if (type == "usb") {
                    iface = new JigInterfacePickit();
                } else {
                }

                iface->setTypeStr(type);
                iface->setAlias(alias);

                switch (iface->getType()) {
                case JigInterface::none:
                    break;
                case JigInterface::app:
                    break;
                case JigInterface::plugin:
                    iface->parameters["path"] = interfaceObject["path"].toString();
                    break;
                case JigInterface::usb:

                    if (iface->getAlias().contains("pickit")) {
                        //      TODO: Test block
                        iface->parameters["target"] = interfaceObject["target"].toString();
                        iface->parameters["mclrPin"] = interfaceObject["mclrPin"].toString();
                        iface->parameters["vccPin"] = interfaceObject["vccPin"].toString();
                        iface->parameters["gndPin"] = interfaceObject["gndPin"].toString();
                        iface->parameters["pgdPin"] = interfaceObject["pgdPin"].toString();
                        iface->parameters["pgcPin"] = interfaceObject["pgcPin"].toString();
                        iface->parameters["bur"] = interfaceObject["bur"].toString();

                        pickitTarget = interfaceObject["target"].toString();
                        pickitMclrPinName = interfaceObject["mclrPin"].toString();
                        pickitVccPinName = interfaceObject["vccPin"].toString();
                        pickitGndPinName = interfaceObject["gndPin"].toString();
                        pickitPgdPinName = interfaceObject["pgdPin"].toString();
                        pickitPgcPinName = interfaceObject["pgcPin"].toString();
                        pickitSerialNumber = interfaceObject["bur"].toString();
                    }
                    break;
                case JigInterface::tty:
                    if(!interfaceObject["vid"].toString().isEmpty()){
                        iface->parameters["vid"] = interfaceObject["vid"].toString();
                    }

                    if(!interfaceObject["pid"].toString().isEmpty()){
                        iface->parameters["pid"] = interfaceObject["pid"].toString();
                    }

                    iface->parameters["portName"] = interfaceObject["defaultPortName"].toString();
                    iface->parameters["baudRate"] = interfaceObject["baudRate"].toInt();
                    iface->parameters["dataBits"] = interfaceObject["dataBits"].toInt();
                    iface->parameters["parity"] = interfaceObject["parity"].toInt();
                    iface->parameters["stopBits"] = interfaceObject["stopBits"].toInt();
                    break;
                }
                iface->config();
                interfaces[alias] = iface;
            }
        }
    }

    if (json.contains("ipeConfigs") && json["ipeConfigs"].isObject()) {
        QJsonObject configsObject = json["ipeConfigs"].toObject();

        configIpePath = configsObject["ipePath"].toString();
        configHexPath = configsObject["hexPath"].toString();
    }

    if (json.contains("commands") && json["commands"].isArray()) {
        QJsonArray commandsArray = json["commands"].toArray();

        for (arrayIterator = commandsArray.begin(); arrayIterator < commandsArray.end();
             arrayIterator++) {
            QJsonObject command = arrayIterator->toObject();

            JigCommand *newCommand = new JigCommand(this);

            newCommand->setId(command["id"].toString());
            newCommand->setCodeName(command["ifCodeName"].toString());
            newCommand->setDescription(command["description"].toString());
            newCommand->runOnJump = command["runOnJump"].toBool();
            newCommand->retry = command["retry"].toBool();
            newCommand->end = command["end"].toBool();
            newCommand->setTimeOut(command["timeout"].toInt());
            newCommand->setTimeDelay(command["delay"].toInt());
            newCommand->setEnabled(command["enabled"].toBool());

            newCommand->setInterfaceName(command["ifName"].toString());
            newCommand->setInterfaceCommand(command["ifCommand"].toString());
            newCommand->setInterfaceArguments(command["ifParameter"].toString());
            newCommand->setInterfaceAnswer(command["ifAnswer"].toString());

            newCommand->messageNotice = command["messageNotice"].toString();
            newCommand->messageOnOk = command["messageOnOk"].toString();
            newCommand->messageOnError = command["messageOnErr"].toString();

            //      USART
            newCommand->setTxCommand(command["txCommand"].toString());
            newCommand->setTxArguments(command["txParameter"].toString());
            newCommand->setRxCommand(command["rxCommand"].toString());
            newCommand->setRxAnswers(command["rxParameter"].toString());
            newCommand->setOnOk(command["onOk"].toString());
            newCommand->setOnError(command["onErr"].toString());

            newCommand->setIsCr(command["CR"].toBool());
            newCommand->setIsLf(command["LF"].toBool());
            newCommand->setIsCrc16(command["CRC16"].toBool());
            newCommand->setIsOptional(command["OPT"].toBool());

            newCommand->setMeasureParameters(command["mean"].toDouble(),
                                             command["deviation"].toDouble(),command["offset"].toDouble());


            newCommand->setMeanFormula(command["formula"].toString());
            if (!newCommand->getMeanFormula().isEmpty()) {
                newCommand->setUseMeanFormula(true);
            } else {
                newCommand->setUseMeanFormula(false);
            }

            newCommand->setMeasFormula(command["measFormula"].toString());

            if (commandList.indexOf(newCommand->getInterfaceCommand()) < 0) {
                error = QString("La prueba #%1:\"%2\" contiene un comando que no existe en la "
                                "tabla de comandos")
                            .arg(arrayIterator.i + 1)
                            .arg(newCommand->getCodeName());
                retCode = -2;
            }

            commands << newCommand;
        }
    }

    return retCode;
}

int JigSaw::writeProfile(QByteArray *data)
{
    QJsonObject json;
    QJsonArray jsonInterfaces;
    QJsonArray jsonCommands;

    foreach (JigInterface *interface, interfaces) {
        QJsonObject jsonInterfaceObject;

        jsonInterfaceObject["alias"] = interface->getAlias();

        switch (interface->getType()) {
        case JigInterface::none:
            break;
        case JigInterface::plugin:
            jsonInterfaceObject["type"] = "plugin";
            jsonInterfaceObject["path"] = interface->parameters["path"].toString();
            break;
        case JigInterface::app:
            jsonInterfaceObject["type"] = "app";
            break;
        case JigInterface::usb: /* Pickit interface */
            if (!interface->getAlias().compare("pickit")) {
                jsonInterfaceObject["type"] = "usb";
                jsonInterfaceObject["target"] = interface->parameters["target"].toString();
                jsonInterfaceObject["bur"] = interface->parameters["bur"].toString();
                jsonInterfaceObject["mclrPin"] = interface->parameters["mclrPin"].toString();
                jsonInterfaceObject["vccPin"] = interface->parameters["vccPin"].toString();
                jsonInterfaceObject["gndPin"] = interface->parameters["gndPin"].toString();
                jsonInterfaceObject["pgdPin"] = interface->parameters["pgdPin"].toString();
                jsonInterfaceObject["pgcPin"] = interface->parameters["pgcPin"].toString();
            }
            break;
        case JigInterface::tty:
            jsonInterfaceObject["type"] = "tty";

            if (interface->parameters["isVidPid"].toBool()) {
                jsonInterfaceObject["vid"] = interface->parameters["vid"].toString();
                jsonInterfaceObject["pid"] = interface->parameters["pid"].toString();
            } else {
                jsonInterfaceObject["defaultPortName"] = interface->parameters["portName"]
                                                             .toString();
            }

            jsonInterfaceObject["baudRate"] = interface->parameters["baudRate"].toInt();
            jsonInterfaceObject["dataBits"] = interface->parameters["dataBits"].toInt();
            jsonInterfaceObject["parity"] = interface->parameters["parity"].toInt();
            jsonInterfaceObject["stopBits"] = interface->parameters["stopBits"].toInt();
            break;
        }

        jsonInterfaces.append(jsonInterfaceObject);
    }

    foreach (JigCommand *cmd, commands) {
        QJsonObject jsonCmd;

        jsonCmd["id"] = cmd->getId();
        jsonCmd["ifCodeName"] = cmd->getCodeName();
        jsonCmd["description"] = cmd->getDescription();

        jsonCmd["ifName"] = cmd->getInterfaceName();
        jsonCmd["ifCommand"] = cmd->getInterfaceCommand();
        if (!cmd->getInterfaceArguments().isEmpty())
            jsonCmd["ifParameter"] = cmd->getInterfaceArguments();
        if (!cmd->getInterfaceAnswer().isEmpty())
            jsonCmd["ifAnswer"] = cmd->getInterfaceAnswer();

        if (!cmd->messageNotice.isEmpty())
            jsonCmd["messageNotice"] = cmd->messageNotice;
        if (!cmd->messageOnOk.isEmpty())
            jsonCmd["messageOnOk"] = cmd->messageOnOk;
        if (!cmd->messageOnError.isEmpty())
            jsonCmd["messageOnErr"] = cmd->messageOnError;

        jsonCmd["enabled"] = cmd->getEnabled();
        if (cmd->getTimeOut() > 0)
            jsonCmd["timeout"] = cmd->getTimeOut();
        if (cmd->getTimeDelay() > 0)
            jsonCmd["delay"] = cmd->getTimeDelay();
        if (cmd->end)
            jsonCmd["end"] = cmd->end;
        if (cmd->runOnJump)
            jsonCmd["runOnJump"] = cmd->runOnJump;

        if (cmd->isTxCommand())
            jsonCmd["txCommand"] = cmd->getTxCommand();
        if (cmd->isRxCommand())
            jsonCmd["rxCommand"] = cmd->getRxCommand();
        if (cmd->isTxArguments())
            jsonCmd["txParameter"] = cmd->getTxArguments();
        if (cmd->isRxAnswers())
            jsonCmd["rxParameter"] = cmd->getRxAnswers();
        if (!cmd->getMeanFormula().isEmpty()){
            jsonCmd["formula"] = cmd->getMeanFormula();
            jsonCmd["useFormula"] = true;
        }
        if (!cmd->getMeasFormula().isEmpty()){
            jsonCmd["measFormula"] = cmd->getMeasFormula();
        }
        if ((cmd->mean > 0.0) && (cmd->getMeanFormula().isEmpty()))
            jsonCmd["mean"] = cmd->mean;
        if (cmd->deviation > 0.0)
            jsonCmd["deviation"] = cmd->deviation;
        if (cmd->offset != 0.0)
            jsonCmd["offset"] = cmd->offset;
        if (!cmd->getOnOk().isEmpty())
            jsonCmd["onOk"] = cmd->getOnOk();
        if (!cmd->getOnError().isEmpty())
            jsonCmd["onErr"] = cmd->getOnError();
        if (cmd->getIsCr())
            jsonCmd["CR"] = cmd->getIsCr();
        if (cmd->getIsLf())
            jsonCmd["LF"] = cmd->getIsLf();
        if (cmd->getIsCrc16())
            jsonCmd["CRC16"] = cmd->getIsCrc16();
        if (cmd->getIsOptional())
            jsonCmd["OPT"] = cmd->getIsOptional();

        jsonCommands.append(jsonCmd);
    }

    QJsonObject ipeConfigs;
    ipeConfigs["ipePath"] = configIpePath;
    ipeConfigs["hexPath"] = configHexPath;

    json["productName"] = productName;
    json["productCode"] = productCode;
    json["interfaces"] = jsonInterfaces;
    json["ipeConfigs"] = ipeConfigs;
    json["commands"] = jsonCommands;

    QJsonDocument jsonDoc(json);
    data->clear();
    data->append(jsonDoc.toJson());

    return 0;
}

QString JigSaw::getPickitMclrPinName() const
{
    return pickitMclrPinName;
}

void JigSaw::setPickitMclrPinName(const QString &value)
{
    pickitMclrPinName = value;
}

QString JigSaw::getPickitVccPinName() const
{
    return pickitVccPinName;
}

void JigSaw::setPickitVccPinName(const QString &value)
{
    pickitVccPinName = value;
}

QString JigSaw::getPickitGndPinName() const
{
    return pickitGndPinName;
}

void JigSaw::setPickitGndPinName(const QString &value)
{
    pickitGndPinName = value;
}

QString JigSaw::getPickitPgdPinName() const
{
    return pickitPgdPinName;
}

void JigSaw::setPickitPgdPinName(const QString &value)
{
    pickitPgdPinName = value;
}

QString JigSaw::getPickitPgcPinName() const
{
    return pickitPgcPinName;
}

void JigSaw::setPickitPgcPinName(const QString &value)
{
    pickitPgcPinName = value;
}

QString JigSaw::getPickitSerialNumber() const
{
    return pickitSerialNumber;
}

void JigSaw::setPickitSerialNumber(const QString &value)
{
    pickitSerialNumber = value;
}

QString JigSaw::getConfigIpePath() const
{
    return configIpePath;
}

void JigSaw::setConfigIpePath(const QString &value)
{
    configIpePath = value;
}

QString JigSaw::getConfigHexPath() const
{
    return configHexPath;
}

void JigSaw::setConfigHexPath(const QString &value)
{
    configHexPath = value;
}

QString JigSaw::getProductName() const
{
    return productName;
}

void JigSaw::setProductName(const QString &value)
{
    productName = value;
}

QString JigSaw::getProductCode() const
{
    return productCode;
}

void JigSaw::setProductCode(const QString &value)
{
    productCode = value;
}

QString JigSaw::getPickitTarget() const
{
    return pickitTarget;
}

void JigSaw::setPickitTarget(const QString &value)
{
    pickitTarget = value;
}
