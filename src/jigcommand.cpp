#include "jigcommand.h"
#include <QDebug>

JigCommand::JigCommand(QObject *parent)
    : QObject(parent)
{
    Qt::ItemFlags flags;

    commandItem = new QTableWidgetItem("");
    flags = commandItem->flags();
    flags = flags & ~Qt::ItemIsEditable;
    commandItem->setFlags(flags);
    commandItem->setToolTip("Command description");
    commandItem->setCheckState(Qt::Unchecked);

    minItem = new QTableWidgetItem("0.00");
    minItem->setTextAlignment(Qt::AlignCenter);
    flags = minItem->flags();
    flags = flags & ~Qt::ItemIsEditable;
    minItem->setFlags(flags);

    measuredItem = new QTableWidgetItem("0.00");
    measuredItem->setTextAlignment(Qt::AlignCenter);
    flags = measuredItem->flags();
    flags = flags & ~Qt::ItemIsEditable;
    measuredItem->setFlags(flags);

    maxItem = new QTableWidgetItem("0.00");
    maxItem->setTextAlignment(Qt::AlignCenter);
    flags = maxItem->flags();
    flags = flags & ~Qt::ItemIsEditable;
    maxItem->setFlags(flags);

    resultItem = new QTableWidgetItem("Pendiente");
    resultItem->setTextAlignment(Qt::AlignCenter);
    flags = resultItem->flags();
    flags = flags & ~Qt::ItemIsEditable;
    resultItem->setFlags(flags);

    ifName = "";
    ifCommand = "";
    ifArgs = "";
    ifAnswer = "";

    setTxCommand("");
    setTxArguments("");

    setRxCommand("");
    setRxAnswers("");

    setMeasureRange(0.0, 0.0);

    onOk = "";
    messageOnOk = "";

    onError = "";
    messageOnError = "";

    messageNotice = "";

    retry = false;
    end = false;
    timeOut = 0.0;
    timeDelay = 0.0;
    enabled = false;
    id = "";
    setDescription("");

    runOnJump = false;
}

JigCommand::JigCommand(QObject *parent, const JigCommand &cmd)
    : QObject(parent)
{
    commandItem = new QTableWidgetItem("");
    minItem = new QTableWidgetItem("0.00");
    measuredItem = new QTableWidgetItem("0.00");
    maxItem = new QTableWidgetItem("0.00");
    resultItem = new QTableWidgetItem("Pendiente");

    ifName = cmd.ifName;
    ifCommand = cmd.ifCommand;
    ifArgs = cmd.ifArgs;
    ifAnswer = cmd.ifAnswer;

    setTxCommand(cmd.txCommand);
    setTxArguments(cmd.txArguments);

    setRxCommand(cmd.rxCommand);
    setRxAnswers(cmd.rxAnswers);

    onOk = cmd.onOk;
    onError = cmd.onError;
    messageOnOk = cmd.messageOnOk;
    messageOnError = cmd.messageOnError;
    messageNotice = cmd.messageNotice;
    retry = cmd.retry;
    end = cmd.end;
    timeOut = cmd.timeOut;
    timeDelay = cmd.timeDelay;
    enabled = cmd.enabled;
    codeName = cmd.codeName;
    id = cmd.id;
    setDescription(cmd.description);
    runOnJump = cmd.runOnJump;

    setMeasureParameters(cmd.mean, cmd.deviation, cmd.offset);

    *commandItem = *cmd.commandItem;
    *minItem = *cmd.minItem;
    *measuredItem = *cmd.measuredItem;
    *maxItem = *cmd.maxItem;
    *resultItem = *cmd.resultItem;
}

JigCommand &JigCommand::operator=(const JigCommand &other)
{
    QObject *parent = other.parent();
    this->~JigCommand();
    new (this) JigCommand(parent, other);
    return *this;
}

QString JigCommand::getMeanFormula() const
{
    return measMeanFormula;
}

void JigCommand::setMeanFormula(const QString &value)
{
    measMeanFormula = value;
}

QString JigCommand::getMeasFormula() const
{
    return measFormula;
}

void JigCommand::setMeasFormula(const QString &value)
{
    measFormula = value;
}

bool JigCommand::getUseMeanFormula() const
{
    return useMeanFormula;
}

void JigCommand::setUseMeanFormula(bool value)
{
    useMeanFormula = value;
}

bool JigCommand::getIsCr() const
{
    return isCr;
}

void JigCommand::setIsCr(bool value)
{
    isCr = value;
}

bool JigCommand::getIsLf() const
{
    return isLf;
}

void JigCommand::setIsLf(bool value)
{
    isLf = value;
}

bool JigCommand::getIsCrc16() const
{
    return isCrc16;
}

void JigCommand::setIsCrc16(bool value)
{
    isCrc16 = value;
}

bool JigCommand::getIsOptional() const
{
    return isOptional;
}

void JigCommand::setIsOptional(bool value)
{
    isOptional = value;
}

QString JigCommand::getInterfaceName() const
{
    return ifName;
}

void JigCommand::setInterfaceName(const QString &value)
{
    ifName = value;
}

QString JigCommand::getInterfaceCommand() const
{
    return ifCommand;
}

void JigCommand::setInterfaceCommand(const QString &value)
{
    ifCommand = value;
}

QString JigCommand::getInterfaceArguments() const
{
    return ifArgs;
}

void JigCommand::setInterfaceArguments(const QString &value)
{
    ifArgs = value;
}

QString JigCommand::getTxCommand() const
{
    return txCommand;
}

void JigCommand::setTxCommand(const QString &value)
{
    txCommand = value;
}

QString JigCommand::getTxArguments() const
{
    return txArguments;
}

void JigCommand::setTxArguments(const QString &value)
{
    txArguments = value;
}

QString JigCommand::getRxCommand() const
{
    return rxCommand;
}

void JigCommand::setRxCommand(const QString &value)
{
    rxCommand = value;
}

QString JigCommand::getRxAnswers() const
{
    return rxAnswers;
}

void JigCommand::setRxAnswers(const QString &value)
{
    rxAnswers = value;
}

QString JigCommand::getOnOk() const
{
    return onOk;
}

void JigCommand::setOnOk(const QString &value)
{
    onOk = value;
}

QString JigCommand::getOnError() const
{
    return onError;
}

void JigCommand::setOnError(const QString &value)
{
    onError = value;
}

bool JigCommand::getEnabled() const
{
    return enabled;
}

void JigCommand::setEnabled(bool value)
{
    enabled = value;

    if (enabled) {
        commandItem->setCheckState(Qt::Checked);
    } else {
        commandItem->setCheckState(Qt::Unchecked);
    }
}

QString JigCommand::getId() const
{
    return id;
}

void JigCommand::setId(const QString &value)
{
    id = value;
}

QString JigCommand::getCodeName() const
{
    return codeName;
}

void JigCommand::setCodeName(const QString &value)
{
    codeName = value;
    this->commandItem->setText(codeName);
}

QString JigCommand::getDescription() const
{
    return description;
}

void JigCommand::setDescription(const QString &value)
{
    description = value;
    this->commandItem->setToolTip(description);
}

int JigCommand::getTimeOut() const
{
    return timeOut;
}

void JigCommand::setTimeOut(int value)
{
    timeOut = value;
}

int JigCommand::getTimeDelay() const
{
    return timeDelay;
}

void JigCommand::setTimeDelay(int value)
{
    timeDelay = value;
}

QString JigCommand::getInterfaceAnswer() const
{
    return ifAnswer;
}

void JigCommand::setInterfaceAnswer(const QString &value)
{
    ifAnswer = value;
}

JigCommand::JigCommandState JigCommand::getState() const
{
    return state;
}

void JigCommand::setState(const JigCommandState &value)
{
    state = value;

    this->resultItem->setTextColor("black");

    switch (state) {
    case pending:
        resultItem->setText("Pendiente");
        break;
    case delay:
        resultItem->setText("Retardo");
        break;
    case running:
        resultItem->setText("En ejecución");
        break;
    case jump:
        resultItem->setText("Salta");
        resultItem->setTextColor("blue");
        break;
    case ok:
        resultItem->setText("OK");
        resultItem->setTextColor("green");
        break;
    case fail:
        resultItem->setText("ERROR");
        resultItem->setTextColor("red");
        break;
    }
}

bool JigCommand::isTxCommand()
{
    return !txCommand.isEmpty();
}

bool JigCommand::isRxCommand()
{
    return !rxCommand.isEmpty();
}

bool JigCommand::isTxArguments()
{
    return !txArguments.isEmpty();
}

bool JigCommand::isRxAnswers()
{
    return !rxAnswers.isEmpty();
}

void JigCommand::setMeasureRange(double min, double max)
{
    this->min = min;
    this->max = max;
    minItem->setText(QString::number(min, 'f', 2));
    maxItem->setText(QString::number(max, 'f', 2));

    this->mean = (max + min) / 2;
    this->deviation = max - this->mean;
}

void JigCommand::setMeasureParameters(double mean, double deviation, double offset)
{
    this->mean = mean;
    this->deviation = deviation;
    this->offset = offset;

    this->max = mean + deviation;
    this->min = mean - deviation;

    minItem->setText(QString::number(this->min, 'f', 2));
    maxItem->setText(QString::number(this->max, 'f', 2));
}

bool JigCommand::setMeasure(double value)
{
    double meas = value + this->offset;

    this->measuredItem->setText(QString::number(meas, 'f', 2));

    if ((this->min < meas) && (meas < this->max)) {
        this->measuredItem->setTextColor("green");
        return true;
    } else {
        this->measuredItem->setTextColor("red");
        return false;
    }
}

void JigCommand::clearMeasure()
{
    this->measuredItem->setText("0.00");
    this->measuredItem->setTextColor("black");
}

bool JigCommand::isOnOk()
{
    return !this->onOk.isEmpty();
}

bool JigCommand::isOnError()
{
    return !this->onError.isEmpty();
}
