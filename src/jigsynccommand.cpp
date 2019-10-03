#include "jigsynccommand.h"
#include <QDebug>

JigSyncCommand::JigSyncCommand(QObject *parent)
    : QObject(parent)
{
    Qt::ItemFlags flags;

    nameItem = new QTableWidgetItem("");
    flags = nameItem->flags();
    flags = flags & ~Qt::ItemIsEditable;
    nameItem->setFlags(flags);
    nameItem->setToolTip("Command description");
    nameItem->setCheckState(Qt::Unchecked);

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

JigSyncCommand::JigSyncCommand(QObject *parent, const JigSyncCommand &cmd)
    : QObject(parent)
{
    nameItem = new QTableWidgetItem("");
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
    name = cmd.name;
    id = cmd.id;
    setDescription(cmd.description);
    runOnJump = cmd.runOnJump;

    setMeasureParameters(cmd.mean, cmd.deviation, cmd.offset);

    *nameItem = *cmd.nameItem;
    *minItem = *cmd.minItem;
    *measuredItem = *cmd.measuredItem;
    *maxItem = *cmd.maxItem;
    *resultItem = *cmd.resultItem;
}

JigSyncCommand &JigSyncCommand::operator=(const JigSyncCommand &other)
{
    QObject *parent = other.parent();
    this->~JigSyncCommand();
    new (this) JigSyncCommand(parent, other);
    return *this;
}

QString JigSyncCommand::getMeanFormula() const
{
    return measMeanFormula;
}

void JigSyncCommand::setMeanFormula(const QString &value)
{
    measMeanFormula = value;
}

QString JigSyncCommand::getMeasFormula() const
{
    return measFormula;
}

void JigSyncCommand::setMeasFormula(const QString &value)
{
    measFormula = value;
}

bool JigSyncCommand::getUseMeanFormula() const
{
    return useMeanFormula;
}

void JigSyncCommand::setUseMeanFormula(bool value)
{
    useMeanFormula = value;
}

bool JigSyncCommand::getIsCr() const
{
    return isCr;
}

void JigSyncCommand::setIsCr(bool value)
{
    isCr = value;
}

bool JigSyncCommand::getIsLf() const
{
    return isLf;
}

void JigSyncCommand::setIsLf(bool value)
{
    isLf = value;
}

bool JigSyncCommand::getIsCrc16() const
{
    return isCrc16;
}

void JigSyncCommand::setIsCrc16(bool value)
{
    isCrc16 = value;
}

bool JigSyncCommand::getIsOptional() const
{
    return isOptional;
}

void JigSyncCommand::setIsOptional(bool value)
{
    isOptional = value;
}

QString JigSyncCommand::getInterfaceName() const
{
    return ifName;
}

void JigSyncCommand::setInterfaceName(const QString &value)
{
    ifName = value;
}

QString JigSyncCommand::getInterfaceCommand() const
{
    return ifCommand;
}

void JigSyncCommand::setInterfaceCommand(const QString &value)
{
    ifCommand = value;
}

QString JigSyncCommand::getInterfaceArguments() const
{
    return ifArgs;
}

void JigSyncCommand::setInterfaceArguments(const QString &value)
{
    ifArgs = value;
}

QString JigSyncCommand::getTxCommand() const
{
    return txCommand;
}

void JigSyncCommand::setTxCommand(const QString &value)
{
    txCommand = value;
}

QString JigSyncCommand::getTxArguments() const
{
    return txArguments;
}

void JigSyncCommand::setTxArguments(const QString &value)
{
    txArguments = value;
}

QString JigSyncCommand::getRxCommand() const
{
    return rxCommand;
}

void JigSyncCommand::setRxCommand(const QString &value)
{
    rxCommand = value;
}

QString JigSyncCommand::getRxAnswers() const
{
    return rxAnswers;
}

void JigSyncCommand::setRxAnswers(const QString &value)
{
    rxAnswers = value;
}

QString JigSyncCommand::getOnOk() const
{
    return onOk;
}

void JigSyncCommand::setOnOk(const QString &value)
{
    onOk = value;
}

QString JigSyncCommand::getOnError() const
{
    return onError;
}

void JigSyncCommand::setOnError(const QString &value)
{
    onError = value;
}

bool JigSyncCommand::getEnabled() const
{
    return enabled;
}

void JigSyncCommand::setEnabled(bool value)
{
    enabled = value;

    if (enabled) {
        nameItem->setCheckState(Qt::Checked);
    } else {
        nameItem->setCheckState(Qt::Unchecked);
    }
}

QString JigSyncCommand::getId() const
{
    return id;
}

void JigSyncCommand::setId(const QString &value)
{
    id = value;
}

QString JigSyncCommand::getName() const
{
    return name;
}

void JigSyncCommand::setName(const QString &value)
{
    this->name = value;
    this->nameItem->setText(value);
}

QString JigSyncCommand::getDescription() const
{
    return description;
}

void JigSyncCommand::setDescription(const QString &value)
{
    description = value;
    this->nameItem->setToolTip(description);
}

int JigSyncCommand::getTimeOut() const
{
    return timeOut;
}

void JigSyncCommand::setTimeOut(int value)
{
    timeOut = value;
}

int JigSyncCommand::getTimeDelay() const
{
    return timeDelay;
}

void JigSyncCommand::setTimeDelay(int value)
{
    timeDelay = value;
}

QString JigSyncCommand::getInterfaceAnswer() const
{
    return ifAnswer;
}

void JigSyncCommand::setInterfaceAnswer(const QString &value)
{
    ifAnswer = value;
}

JigSyncCommand::JigCommandState JigSyncCommand::getState() const
{
    return state;
}

void JigSyncCommand::setState(const JigCommandState &value)
{
    state = value;

    this->resultItem->setTextColor("white");

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

bool JigSyncCommand::isTxCommand()
{
    return !txCommand.isEmpty();
}

bool JigSyncCommand::isRxCommand()
{
    return !rxCommand.isEmpty();
}

bool JigSyncCommand::isTxArguments()
{
    return !txArguments.isEmpty();
}

bool JigSyncCommand::isRxAnswers()
{
    return !rxAnswers.isEmpty();
}

void JigSyncCommand::setMeasureRange(double min, double max)
{
    this->min = min;
    this->max = max;
    minItem->setText(QString::number(min, 'f', 2));
    maxItem->setText(QString::number(max, 'f', 2));

    this->mean = (max + min) / 2;
    this->deviation = max - this->mean;
}

void JigSyncCommand::setMeasureParameters(double mean, double deviation, double offset)
{
    this->mean = mean;
    this->deviation = deviation;
    this->offset = offset;

    this->max = mean + deviation;
    this->min = mean - deviation;

    minItem->setText(QString::number(this->min, 'f', 2));
    maxItem->setText(QString::number(this->max, 'f', 2));
}

bool JigSyncCommand::setMeasure(double value)
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

void JigSyncCommand::clearMeasure()
{
    this->measuredItem->setText("0.00");
    this->measuredItem->setTextColor("white");
}

bool JigSyncCommand::isOnOk()
{
    return !this->onOk.isEmpty();
}

bool JigSyncCommand::isOnError()
{
    return !this->onError.isEmpty();
}
