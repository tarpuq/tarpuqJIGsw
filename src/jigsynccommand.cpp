#include "jigsynccommand.h"
#include <QDebug>

#include <QJSEngine>
#include <QDateTime>

JigSyncCommand::JigSyncCommand(QObject *parent)
    : QObject(parent)
{
    Qt::ItemFlags flags;

    treeItem = new QTreeWidgetItem();
    treeItem->setText(colTest,"");
    treeItem->setToolTip(colTest,"Command description");
    treeItem->setCheckState(colTest,Qt::Unchecked);

    treeItem->setText(colMin,"0.00");
    treeItem->setTextAlignment(colMin,Qt::AlignCenter);

    treeItem->setText(colMeasure,"0.00");
    treeItem->setTextAlignment(colMeasure,Qt::AlignCenter);

    treeItem->setText(colMax,"0.00");
    treeItem->setTextAlignment(colMax,Qt::AlignCenter);

    treeItem->setText(colState,"Pendiente");
    treeItem->setTextAlignment(colState,Qt::AlignCenter);

    ifName = "";
    ifCommand = "";
    ifArguments = "";
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

    isCr = false;
    isLf = false;
    isCrc16 = false;
    isOptional = false;

    runOnJump = false;
}

JigSyncCommand::JigSyncCommand(QObject *parent, const JigSyncCommand &cmd)
    : QObject(parent)
{
    treeItem = new QTreeWidgetItem();

    ifName = cmd.ifName;
    ifCommand = cmd.ifCommand;
    ifArguments = cmd.ifArguments;
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

    isCr = cmd.isCr;
    isLf = cmd.isLf;
    isCrc16 = cmd.isCrc16;
    isOptional = cmd.isOptional;

    setMeasureParameters(cmd.mean, cmd.deviation, cmd.offset);

    *treeItem = *cmd.treeItem;
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

void JigSyncCommand::refreshState()
{
    int state = treeItem->checkState(colTest);

    if(state == Qt::Checked){
        this->setEnabled(true);
    } else if (state == Qt::Unchecked){
        this->setEnabled(false);
    }
}

void JigSyncCommand::setMeasureError(QString value)
{
    QBrush brush = treeItem->foreground(colMeasure);
    treeItem->setText(colMeasure,value);
    brush.setColor(Qt::red);
    treeItem->setForeground(colMeasure,brush);
}

QByteArray JigSyncCommand::buildFrameToSend(QHash<QString, QByteArray> *wildcard)
{
    QByteArray ans;
    QByteArray builded;

    if (!this->getTxCommand().isEmpty()) {
        //        Wildcard for random variables
        if (this->getTxCommand().contains("%")) {
            builded = wildcard->value(this->getTxCommand());
        } else {
            builded = this->getTxCommand().toLatin1();
        }

        if (this->isTxArguments()) {
            builded.append(" ");

            //        Wildcard for random variables
            if (this->getTxArguments().contains("%")) {
                builded.append(wildcard->value(this->getTxArguments()));
            } else {
                builded.append(this->getTxArguments());
            }
        }
    }

    return builded;
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
    return ifArguments;
}

void JigSyncCommand::setInterfaceArguments(const QString &value)
{
    ifArguments = value;
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
        treeItem->setCheckState(0,Qt::Checked);
    } else {
        treeItem->setCheckState(0,Qt::Unchecked);
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
    this->treeItem->setText(0,value);
}

QString JigSyncCommand::getDescription() const
{
    return description;
}

void JigSyncCommand::setDescription(const QString &value)
{
    description = value;
    treeItem->setToolTip(colTest,description);
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

void JigSyncCommand::setStatus(const JigCommandState &value)
{
    state = value;
    setStatusItem(treeItem, state);
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

    this->mean = (max + min) / 2;
    this->deviation = max - this->mean;

    treeItem->setText(colMin,QString::number(this->min, 'f', 2));
    treeItem->setText(colMax,QString::number(this->max, 'f', 2));
}

void JigSyncCommand::setMeasureParameters(double mean, double deviation, double offset)
{
    this->mean = mean;
    this->deviation = deviation;
    this->offset = offset;

    this->max = mean + deviation;
    this->min = mean - deviation;

    treeItem->setText(colMin,QString::number(this->min, 'f', 2));
    treeItem->setText(colMax,QString::number(this->max, 'f', 2));
}

int JigSyncCommand::processMeasure(QTreeWidgetItem *item, QString value)
{
    QString formula;
    QJSValue evaluator;
    QJSEngine jsEngine;
    double measureValue;

    formula = this->getMeasFormula();

    if (!formula.isEmpty()){
        if (formula.contains("%")) {      //  Wildcard
            if (formula.contains("%f")) { // floating
                formula.replace("%f", value);
            }
        }

        evaluator = jsEngine.evaluate(formula);
        measureValue = evaluator.toNumber();
    } else {
        measureValue = value.toDouble();
    }

    return this->setMeasureItem(item,measureValue);
}

int JigSyncCommand::setMeasureItem(QTreeWidgetItem *item, double value)
{
    QBrush brush = item->foreground(colMeasure);
    int err = 0;

    double meas = value + this->offset;

    item->setText(colMeasure,QString::number(meas, 'f', 2));
    item->setTextAlignment(colMeasure, Qt::AlignHCenter);

    if ((this->min < meas) && (meas < this->max)) {
        brush.setColor(Qt::green);
    } else {
        brush.setColor(Qt::red);
        err = 1;
    }

    item->setForeground(colMeasure,brush);
    return err;
}

bool JigSyncCommand::compareDate(QString date)
{
    QDateTime t1 = QDateTime::fromString(date, "dd/MM/yyyy HH:mm:ss");
    QDateTime t2 = QDateTime::currentDateTime();

    qDebug() << "localTime" << t1 << "; deviceTime" << t2
             << "diff:" << t2.secsTo(t1);

    if (abs(t2.secsTo(t1)) < 10) {
        return true;
    } else {
        return false;
    }
}

void JigSyncCommand::setStatusItem(QTreeWidgetItem *item, JigSyncCommand::JigCommandState status)
{
    QBrush brush = item->foreground(colState);
    QString strStatus;
    brush.setColor(Qt::white);

    switch (status) {
    case pending:
        strStatus = "Pendiente";
        break;
    case delay:
        strStatus = "Retardo";
        break;
    case running:
        strStatus = "En ejecución";
        break;
    case expand:
        strStatus = "Desplegado";
        break;
    case jump:
        strStatus = "Salta";
        brush.setColor(Qt::blue);
        break;
    case ok:
        strStatus = "OK";
        brush.setColor(Qt::green);
        break;
    case fail:
        strStatus = "ERROR";
        brush.setColor(Qt::red);
        break;
    }

    item->setText(colState,strStatus);
    item->setTextAlignment(colState,Qt::AlignHCenter);
    item->setForeground(colState,brush);
}

void JigSyncCommand::clearMeasure()
{
    QBrush brush = this->treeItem->foreground(colMeasure);
    treeItem->takeChildren();
    treeItem->setText(colMeasure,"0.00");
    brush.setColor(Qt::white);
    treeItem->setForeground(colMeasure,brush);
}

bool JigSyncCommand::isOnOk()
{
    return !this->onOk.isEmpty();
}

bool JigSyncCommand::isOnError()
{
    return !this->onError.isEmpty();
}

int JigSyncCommand::processAnswers(QStringList answers, QHash<QString, QByteArray> *wildcard, int index, QList<bool> *flags, QStringList *report, QList<bool> f_jump)
{
    QTreeWidgetItem *item;
    int err = 0;
    int status;
    int i = 0;

    QString prefix = "";

    if(answers.isEmpty())
        return 1;

    this->treeItem->takeChildren();

    foreach(QString ansX, answers){
        status = 1;

        *flags << false;

        if(!f_jump.at(i)){
            //  Check if answer is array
            if(answers.size() > 1){
                this->setStatus(JigCommandState::expand);
                this->treeItem->setText(colMeasure, QString::number(this->mean));

                QTreeWidgetItem *child = new QTreeWidgetItem(this->treeItem);
                child->setText(colTest, QString("DUT %1").arg(i + 1));
                child->setTextAlignment(colTest,Qt::AlignLeft);
                child->setText(colMin, this->treeItem->text(colMin));
                child->setTextAlignment(colMin,Qt::AlignCenter);
                child->setText(colMeasure, this->treeItem->text(colMeasure));
                child->setTextAlignment(colMeasure,Qt::AlignCenter);
                child->setText(colMax, this->treeItem->text(colMax));
                child->setTextAlignment(colMax,Qt::AlignCenter);

                prefix = "        ";

                item = child;
            } else {
                prefix = "";

                item = this->treeItem;
            }

            if(rxAnswers.contains("%")){    //  Wildcard
                if(rxAnswers == "%D"){  //  Date
                    status = compareDate(ansX);
                } else if (rxAnswers == "%measure") {   //  Measure
                    status = processMeasure(item, ansX);
                } else if (rxAnswers == "%serialNumber") {   //  Measure
                    if (wildcard->value("%serialNumber") == ansX)
                        status = 0;
                } else if (rxAnswers == "%x") {   //  Hexadecimal value
                    wildcard->insert("%x", QByteArray::fromHex(ansX.toLatin1()));
                    status = 0;
                } else {
                }
            } else {    //  Normal
                if(rxAnswers == ansX){
                    status = 0;
                }
            }

            switch (status) {
            case 0:
                flags->replace(i, false);
                this->setStatusItem(item,JigCommandState::ok);
                break;
            case 1:
                err = 1;

                *report << QString::number(index) + ":" + this->getId();
                *report << this->messageOnError;
                *report << ansX;

                flags->replace(i, true);
                this->setStatusItem(item,JigCommandState::fail);
                break;
            default:
                break;
            }

            item->setText(colTest, item->text(colTest).prepend(prefix));
            item->setText(colMin, item->text(colMin).prepend(prefix));
            item->setText(colMeasure, item->text(colMeasure).prepend(prefix));
            item->setText(colMax, item->text(colMax).prepend(prefix));
            item->setText(colState, item->text(colState).prepend(prefix));
        }

        i++;
    }

    treeItem->setExpanded(true);

    return err;
}
