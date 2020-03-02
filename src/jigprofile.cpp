#include "jigprofile.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <QDebug>
#include <QMessageBox>

JigProfile::JigProfile(QObject *parent)
    : QObject(parent)
{
    productCode = "";
    productName = "";
    isPanelized = false;
    panelRows = 0;
    panelCols = 0;
}

int JigProfile::readProfile(QByteArray data)
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

    syncCommands.clear();
    asyncCommands.clear();
    interfaces.clear();
    availableInterfaces.clear();

    //      Process
    if (json.contains("productName") && json["productName"].isString()) {
        productName = json["productName"].toString();
    }

    if (json.contains("productCode") && json["productCode"].isString()) {
        productCode = json["productCode"].toString();
    }

    if (json.contains("isPanelized") && json["isPanelized"].isBool()) {
        isPanelized = json["isPanelized"].toBool();
    }

    if (json.contains("panelRows")) {
        panelRows = json["panelRows"].toInt();
    }

    if (json.contains("panelCols")) {
        panelCols = json["panelCols"].toInt();
    }

    if (json.contains("hexPath") && json["hexPath"].isString()) {
        fwPath = json["hexPath"].toString();
    }

    panelAmount = panelRows * panelCols;

    if (json.contains("interfaces") && json["interfaces"].isArray()) {
        QJsonArray interfacesArray = json["interfaces"].toArray();

        for (arrayIterator = interfacesArray.begin(); arrayIterator < interfacesArray.end();
             arrayIterator++) {
            QJsonObject interfaceObject = arrayIterator->toObject();

            if (interfaceObject.contains("alias") && interfaceObject["alias"].isString()) {
                QString type = interfaceObject["type"].toString();
                QString alias = interfaceObject["alias"].toString();

                JigInterface *iface = nullptr;

                if (type == "plugin") {
                    iface = new JigInterfacePlugin();
                } else if (type == "tty") {
                    iface = new JigInterfaceTty();
                } else if (type == "usb") {
                    iface = new JigInterfacePickit();
                } else {
                    continue;
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
                        iface->parameters["ipePath"] = interfaceObject["ipePath"].toString();
                    }
                    break;
                case JigInterface::tty:
                    if (!interfaceObject["vid"].toString().isEmpty()) {
                        iface->parameters["vid"] = interfaceObject["vid"].toString();
                    }

                    if (!interfaceObject["pid"].toString().isEmpty()) {
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
                availableInterfaces << alias;
            }
        }

        //  Creating default app interface
        JigInterface *iface = new JigInterface(this);
        QString appAlias = "app";
        iface->setTypeStr(appAlias);
        iface->setAlias(appAlias);
        iface->config();
        interfaces[appAlias] = iface;
        availableInterfaces << appAlias;
    }

    if (json.contains("asyncCommands") && json["asyncCommands"].isArray()) {
        QJsonArray asyncCommandsArray = json["asyncCommands"].toArray();

        for (arrayIterator = asyncCommandsArray.begin(); arrayIterator < asyncCommandsArray.end();
             arrayIterator++) {
            QJsonObject asyncCommand = arrayIterator->toObject();

            JigSyncCommand *newCommand = new JigSyncCommand(this);

            newCommand->setId(asyncCommand["id"].toString());
            newCommand->setName(asyncCommand["name"].toString());
            newCommand->setDescription(asyncCommand["description"].toString());

            newCommand->setInterfaceName(asyncCommand["ifName"].toString());
            newCommand->setInterfaceCommand(asyncCommand["ifCommand"].toString());
            newCommand->setInterfaceArguments(asyncCommand["ifParameter"].toString());
            newCommand->setInterfaceAnswer(asyncCommand["ifAnswer"].toString());

            //      USART
            newCommand->setTxCommand(asyncCommand["txCommand"].toString());
            newCommand->setTxArguments(asyncCommand["txParameter"].toString());

            newCommand->setIsCr(asyncCommand["CR"].toBool());
            newCommand->setIsLf(asyncCommand["LF"].toBool());
            newCommand->setIsCrc16(asyncCommand["CRC16"].toBool());
            newCommand->setIsOptional(asyncCommand["OPT"].toBool());

            if(!isCommandValid(newCommand)){
                error = QString("La prueba #%1:\"%2\" contiene un comando que no existe en la "
                                "tabla de comandos")
                            .arg(arrayIterator.i + 1)
                            .arg(newCommand->getName());
                retCode = -2;
            }

            asyncCommands << newCommand;
        }
    }

    if (json.contains("syncCommands") && json["syncCommands"].isArray()) {
        QJsonArray syncCommandsArray = json["syncCommands"].toArray();

        for (arrayIterator = syncCommandsArray.begin(); arrayIterator < syncCommandsArray.end();
             arrayIterator++) {
            QJsonObject syncCommand = arrayIterator->toObject();

            JigSyncCommand *newCommand = new JigSyncCommand(this);

            newCommand->setId(syncCommand["id"].toString());
            newCommand->setName(syncCommand["name"].toString());
            newCommand->setDescription(syncCommand["description"].toString());
            newCommand->runOnJump = syncCommand["runOnJump"].toBool();
            newCommand->retry = syncCommand["retry"].toBool();
            newCommand->end = syncCommand["end"].toBool();
            newCommand->setTimeOut(syncCommand["timeout"].toInt());
            newCommand->setTimeDelay(syncCommand["delay"].toInt());
            newCommand->setEnabled(syncCommand["enabled"].toBool());

            newCommand->setInterfaceName(syncCommand["ifName"].toString());
            newCommand->setInterfaceCommand(syncCommand["ifCommand"].toString());
            newCommand->setInterfaceArguments(syncCommand["ifParameter"].toString());
            newCommand->setInterfaceAnswer(syncCommand["ifAnswer"].toString());

            newCommand->messageNotice = syncCommand["messageNotice"].toString();
            newCommand->messageOnOk = syncCommand["messageOnOk"].toString();
            newCommand->messageOnError = syncCommand["messageOnErr"].toString();

            //      USART
            newCommand->setTxCommand(syncCommand["txCommand"].toString());
            newCommand->setTxArguments(syncCommand["txParameter"].toString());
            newCommand->setRxCommand(syncCommand["rxCommand"].toString());
            newCommand->setRxAnswers(syncCommand["rxParameter"].toString());
            newCommand->setOnOk(syncCommand["onOk"].toString());
            newCommand->setOnError(syncCommand["onErr"].toString());

            newCommand->setIsCr(syncCommand["CR"].toBool());
            newCommand->setIsLf(syncCommand["LF"].toBool());
            newCommand->setIsCrc16(syncCommand["CRC16"].toBool());
            newCommand->setIsOptional(syncCommand["OPT"].toBool());

            newCommand->setMeasureParameters(syncCommand["mean"].toDouble(),
                                             syncCommand["deviation"].toDouble(),
                                             syncCommand["offset"].toDouble());

            newCommand->setMeanFormula(syncCommand["formula"].toString());
            if (!newCommand->getMeanFormula().isEmpty()) {
                newCommand->setUseMeanFormula(true);
            } else {
                newCommand->setUseMeanFormula(false);
            }

            newCommand->setMeasFormula(syncCommand["measFormula"].toString());

            if(!isCommandValid(newCommand)){
                error = QString("La prueba #%1:\"%2\" contiene un comando que no existe en la "
                                "tabla de comandos")
                            .arg(arrayIterator.i + 1)
                            .arg(newCommand->getName());
                retCode = -2;
            }

            syncCommands << newCommand;
        }
    }

    return retCode;
}

int JigProfile::writeProfile(QByteArray *data)
{
    QJsonObject json;
    QJsonArray jsonInterfaces;
    QJsonArray jsonSyncCommands;
    QJsonArray jsonAsyncCommands;

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
                jsonInterfaceObject["ipePath"] = interface->parameters["ipePath"].toString();
            }
            break;
        case JigInterface::tty:
            jsonInterfaceObject["type"] = "tty";

            if (interface->parameters["isVidPid"].toBool()) {
                jsonInterfaceObject["vid"] = interface->parameters["vid"].toString();
                jsonInterfaceObject["pid"] = interface->parameters["pid"].toString();
            } else {
                jsonInterfaceObject["defaultPortName"] = interface->parameters["portName"].toString();
            }

            jsonInterfaceObject["baudRate"] = interface->parameters["baudRate"].toInt();
            jsonInterfaceObject["dataBits"] = interface->parameters["dataBits"].toInt();
            jsonInterfaceObject["parity"] = interface->parameters["parity"].toInt();
            jsonInterfaceObject["stopBits"] = interface->parameters["stopBits"].toInt();
            break;
        }

        jsonInterfaces.append(jsonInterfaceObject);
    }

    foreach (JigSyncCommand *cmd, syncCommands) {
        QJsonObject jsonSyncCmd;

        jsonSyncCmd["id"] = cmd->getId();
        jsonSyncCmd["name"] = cmd->getName();
        jsonSyncCmd["description"] = cmd->getDescription();

        jsonSyncCmd["ifName"] = cmd->getInterfaceName();
        jsonSyncCmd["ifCommand"] = cmd->getInterfaceCommand();
        if (!cmd->getInterfaceArguments().isEmpty())
            jsonSyncCmd["ifParameter"] = cmd->getInterfaceArguments();
        if (!cmd->getInterfaceAnswer().isEmpty())
            jsonSyncCmd["ifAnswer"] = cmd->getInterfaceAnswer();

        if (!cmd->messageNotice.isEmpty())
            jsonSyncCmd["messageNotice"] = cmd->messageNotice;
        if (!cmd->messageOnOk.isEmpty())
            jsonSyncCmd["messageOnOk"] = cmd->messageOnOk;
        if (!cmd->messageOnError.isEmpty())
            jsonSyncCmd["messageOnErr"] = cmd->messageOnError;

        jsonSyncCmd["enabled"] = cmd->getEnabled();
        if (cmd->getTimeOut() > 0)
            jsonSyncCmd["timeout"] = cmd->getTimeOut();
        if (cmd->getTimeDelay() > 0)
            jsonSyncCmd["delay"] = cmd->getTimeDelay();
        if (cmd->end)
            jsonSyncCmd["end"] = cmd->end;
        if (cmd->runOnJump)
            jsonSyncCmd["runOnJump"] = cmd->runOnJump;

        if (cmd->isTxCommand())
            jsonSyncCmd["txCommand"] = cmd->getTxCommand();
        if (cmd->isRxCommand())
            jsonSyncCmd["rxCommand"] = cmd->getRxCommand();
        if (cmd->isTxArguments())
            jsonSyncCmd["txParameter"] = cmd->getTxArguments();
        if (cmd->isRxAnswers())
            jsonSyncCmd["rxParameter"] = cmd->getRxAnswers();
        if (!cmd->getMeanFormula().isEmpty()) {
            jsonSyncCmd["formula"] = cmd->getMeanFormula();
            jsonSyncCmd["useFormula"] = true;
        }
        if (!cmd->getMeasFormula().isEmpty()) {
            jsonSyncCmd["measFormula"] = cmd->getMeasFormula();
        }
        if ((cmd->mean > 0.0) && (cmd->getMeanFormula().isEmpty()))
            jsonSyncCmd["mean"] = cmd->mean;
        if (cmd->deviation > 0.0)
            jsonSyncCmd["deviation"] = cmd->deviation;
        if (cmd->offset != 0.0)
            jsonSyncCmd["offset"] = cmd->offset;
        if (!cmd->getOnOk().isEmpty())
            jsonSyncCmd["onOk"] = cmd->getOnOk();
        if (!cmd->getOnError().isEmpty())
            jsonSyncCmd["onErr"] = cmd->getOnError();
        if (cmd->getIsCr())
            jsonSyncCmd["CR"] = cmd->getIsCr();
        if (cmd->getIsLf())
            jsonSyncCmd["LF"] = cmd->getIsLf();
        if (cmd->getIsCrc16())
            jsonSyncCmd["CRC16"] = cmd->getIsCrc16();
        if (cmd->getIsOptional())
            jsonSyncCmd["OPT"] = cmd->getIsOptional();

        jsonSyncCommands.append(jsonSyncCmd);
    }

    foreach (JigSyncCommand *cmd, asyncCommands) {
        QJsonObject jsonAsyncCmd;

        jsonAsyncCmd["id"] = cmd->getId();
        jsonAsyncCmd["name"] = cmd->getName();
        jsonAsyncCmd["description"] = cmd->getDescription();

        jsonAsyncCmd["ifName"] = cmd->getInterfaceName();
        jsonAsyncCmd["ifCommand"] = cmd->getInterfaceCommand();
        if (!cmd->getInterfaceArguments().isEmpty())
            jsonAsyncCmd["ifParameter"] = cmd->getInterfaceArguments();
        if (!cmd->getInterfaceAnswer().isEmpty())
            jsonAsyncCmd["ifAnswer"] = cmd->getInterfaceAnswer();

        if (cmd->isTxCommand())
            jsonAsyncCmd["txCommand"] = cmd->getTxCommand();
        if (cmd->isTxArguments())
            jsonAsyncCmd["txParameter"] = cmd->getTxArguments();
        if (cmd->getIsCr())
            jsonAsyncCmd["CR"] = cmd->getIsCr();
        if (cmd->getIsLf())
            jsonAsyncCmd["LF"] = cmd->getIsLf();
        if (cmd->getIsCrc16())
            jsonAsyncCmd["CRC16"] = cmd->getIsCrc16();
        if (cmd->getIsOptional())
            jsonAsyncCmd["OPT"] = cmd->getIsOptional();

        jsonAsyncCommands.append(jsonAsyncCmd);
    }

    json["productName"] = productName;
    json["productCode"] = productCode;
    json["isPanelized"] = isPanelized;
    json["panelRows"] = panelRows;
    json["panelCols"] = panelCols;
    json["interfaces"] = jsonInterfaces;
    json["hexPath"] = fwPath;
    json["syncCommands"] = jsonSyncCommands;
    json["asyncCommands"] = jsonAsyncCommands;

    QJsonDocument jsonDoc(json);
    data->clear();
    data->append(jsonDoc.toJson());

    return 0;
}

QString JigProfile::getFwPath() const
{
    return fwPath;
}

void JigProfile::setFwPath(const QString &value)
{
    fwPath = value;
}

QString JigProfile::getProductName() const
{
    return productName;
}

void JigProfile::setProductName(const QString &value)
{
    productName = value;
}

QString JigProfile::getProductCode() const
{
    return productCode;
}

void JigProfile::setProductCode(const QString &value)
{
    productCode = value;
}

bool JigProfile::getIsPanelized() const
{
    return isPanelized;
}

void JigProfile::setIsPanelized(bool value)
{
    isPanelized = value;
}

int JigProfile::getPanelRows() const
{
    return panelRows;
}

void JigProfile::setPanelRows(int value)
{
    panelRows = value;
}

int JigProfile::getPanelCols() const
{
    return panelCols;
}

void JigProfile::setPanelCols(int value)
{
    panelCols = value;
}

int JigProfile::getPanelAmount() const
{
    return panelAmount;
}

bool JigProfile::isCommandValid(JigSyncCommand *command)
{
    int index = 0;

    switch (interfaces[command->getInterfaceName()]->getType()) {
    case JigInterface::tty:
        index = JigInterfaceTty::getDefaultCommands().indexOf(command->getInterfaceCommand());
        break;
    case JigInterface::usb:
        index = JigInterfacePickit::getDefaultCommands().indexOf(command->getInterfaceCommand());
        break;
    case JigInterface::plugin:
        index = JigInterfacePlugin::getDefaultCommands().indexOf(command->getInterfaceCommand());
        break;
    case JigInterface::app:
        index = JigInterfaceApp::getDefaultCommands().indexOf(command->getInterfaceCommand());
        break;
    default:
        break;
    }

    if (index < 0) {
        return false;
    } else {
        return true;
    }
}

void JigProfile::setPanelAmount(int value)
{
    panelAmount = value;
}
