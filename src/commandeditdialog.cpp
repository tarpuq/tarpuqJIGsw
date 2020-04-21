#include "commandeditdialog.h"
#include "ui_commandeditdialog.h"

#include "jigappinterface.h"
#include "jigpickitinterface.h"
#include "jigplugininterface.h"
#include "jigttyinterface.h"

#include <QDebug>

CommandEditDialog::CommandEditDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CommandEditDialog)
{
    ui->setupUi(this);

    interfacesList = new QStringListModel(this);

    commandsList = new QStringListModel(this);

    setAppCommands(JigAppInterface::getDefaultCommands());
    setTtyCommandsList(JigTtyInterface::getDefaultCommands());
    setUsbCommandsList(JigPickitInterface::getDefaultCommands());
    setPluginCommandsList(JigPluginInterface::getDefaultCommands());

    ui->comboBox_tool->setModel(interfacesList);

    ui->comboBox_command->setModel(commandsList);
}

CommandEditDialog::~CommandEditDialog()
{
    delete ui;
}

void CommandEditDialog::loadCommand(int index, JigSyncCommand *cmd)
{
    this->index = index;
    this->command = cmd;

    ui->comboBox_tool->setCurrentText(command->getInterfaceName());
    ui->comboBox_command->setCurrentText(command->getInterfaceCommand());
    ui->lineEdit_ifArgs->setText(command->getInterfaceArguments());
    ui->lineEdit_ifAns->setText(command->getInterfaceAnswer());
    ui->lineEdit_messageNotice->setText(command->messageNotice);
    ui->checkBox_retry->setChecked(command->retry);
    ui->checkBox_end->setChecked(command->end);
    ui->checkBox_enable->setChecked(command->getEnabled());
    ui->spinBox_timeout->setValue(command->getTimeOut());
    ui->spinBox_delay->setValue(command->getTimeDelay());
    ui->lineEdit_codeName->setText(command->getName());
    ui->lineEdit_id->setText(command->getId());
    ui->lineEdit_description->setText(command->getDescription());
    ui->checkBox_runOnJump->setChecked(command->runOnJump);

    //      USART
    ui->checkBox_cr->setChecked(command->getIsCr());
    ui->checkBox_lf->setChecked(command->getIsLf());
    ui->checkBox_crc16->setChecked(command->getIsCrc16());
    ui->checkBox_optional->setChecked(command->getIsOptional());
    ui->lineEdit_txCommand->setText(command->getTxCommand());
    ui->lineEdit_txArguments->setText(command->getTxArguments());
    ui->lineEdit_rxCommand->setText(command->getRxCommand());
    ui->lineEdit_rxAnswers->setText(command->getRxAnswers());
    ui->lineEdit_onOk->setText(command->getOnOk());
    ui->lineEdit_onError->setText(command->getOnError());
    ui->lineEdit_messageOnOk->setText(command->messageOnOk);
    ui->lineEdit_messageOnError->setText(command->messageOnError);

    //  MEASURE
    ui->doubleSpinBox_deviation->setValue(command->deviation);
    ui->doubleSpinBox_meanFixed->setValue(command->mean);
    ui->doubleSpinBox_offset->setValue(command->offset);
    ui->checkBox_meanFormula->setChecked(command->getUseMeanFormula());
    ui->lineEdit_meanFormula->setText(command->getMeanFormula());
    ui->lineEdit_meanFormula->setEnabled(command->getUseMeanFormula());
    ui->lineEdit_measFormula->setText(command->getMeasFormula());
}

void CommandEditDialog::on_buttonBox_accepted()
{
    command->setInterfaceName(ui->comboBox_tool->currentText());
    command->setInterfaceCommand(ui->comboBox_command->currentText());
    command->setInterfaceArguments(ui->lineEdit_ifArgs->text());
    command->setInterfaceAnswer(ui->lineEdit_ifAns->text());
    command->messageNotice = ui->lineEdit_messageNotice->text();
    command->retry = ui->checkBox_retry->isChecked();
    command->end = ui->checkBox_end->isChecked();
    command->setTimeOut(ui->spinBox_timeout->value());
    command->setTimeDelay(ui->spinBox_delay->value());
    command->setEnabled(ui->checkBox_enable->isChecked());
    command->setName(ui->lineEdit_codeName->text());
    command->setId(ui->lineEdit_id->text());
    command->setDescription(ui->lineEdit_description->text());
    command->runOnJump = ui->checkBox_runOnJump->isChecked();

    //  USART
    command->setIsCr(ui->checkBox_cr->isChecked());
    command->setIsLf(ui->checkBox_lf->isChecked());
    command->setIsCrc16(ui->checkBox_crc16->isChecked());
    command->setIsOptional(ui->checkBox_optional->isChecked());
    command->setTxCommand(ui->lineEdit_txCommand->text());
    command->setTxArguments(ui->lineEdit_txArguments->text());
    command->setRxCommand(ui->lineEdit_rxCommand->text());
    command->setRxAnswers(ui->lineEdit_rxAnswers->text());
    command->setOnOk(ui->lineEdit_onOk->text());
    command->setOnError(ui->lineEdit_onError->text());
    command->messageOnOk = ui->lineEdit_messageOnOk->text();
    command->messageOnError = ui->lineEdit_messageOnError->text();

    //  MEASURE
    command->setUseMeanFormula(ui->checkBox_meanFormula->isChecked());
    command->setMeanFormula(ui->lineEdit_meanFormula->text());
    command->setMeasFormula(ui->lineEdit_measFormula->text());
    command->setMeasureParameters(ui->doubleSpinBox_meanFixed->value(),
                                  ui->doubleSpinBox_deviation->value(),
                                  ui->doubleSpinBox_offset->value());

    command->setStatus(JigSyncCommand::pending);
}

void CommandEditDialog::setInterfaces(QHash<QString, JigAbstractInterface *> *interfaces)
{
    this->interfaces = interfaces;
    interfacesList->setStringList(this->interfaces->keys());
}

void CommandEditDialog::setAppCommands(QStringList commands)
{
    appCommandList = commands;
}

void CommandEditDialog::setTtyCommandsList(const QStringList &value)
{
    ttyCommandList = value;
}

void CommandEditDialog::setUsbCommandsList(const QStringList &value)
{
    usbCommandList = value;
}

void CommandEditDialog::setPluginCommandsList(const QStringList &value)
{
    pluginCommandList = value;
}

void CommandEditDialog::on_doubleSpinBox_meanFixed_valueChanged(double arg1)
{
    double max = arg1 + ui->doubleSpinBox_deviation->value();
    double min = arg1 - ui->doubleSpinBox_deviation->value();

    ui->lineEdit_measMax->setText(QString::number(max));
    ui->lineEdit_measMin->setText(QString::number(min));
}

void CommandEditDialog::on_doubleSpinBox_deviation_valueChanged(double arg1)
{
    double max = ui->doubleSpinBox_meanFixed->value() + arg1;
    double min = ui->doubleSpinBox_meanFixed->value() - arg1;

    ui->lineEdit_measMax->setText(QString::number(max));
    ui->lineEdit_measMin->setText(QString::number(min));
}

void CommandEditDialog::on_checkBox_meanFormula_toggled(bool checked)
{
    ui->lineEdit_meanFormula->setEnabled(checked);
    ui->doubleSpinBox_meanFixed->setEnabled(!checked);
}

void CommandEditDialog::on_comboBox_tool_currentIndexChanged(const QString &arg1)
{
    if(arg1.isEmpty())
        return;

    JigAbstractInterface::JigInterfaceType type = JigAbstractInterface::none;
    type = interfaces->value(arg1)->getType();

    switch (type) {
    case JigAbstractInterface::tty:
        this->commandsList->setStringList(ttyCommandList);
        break;
    case JigAbstractInterface::usb:
        this->commandsList->setStringList(usbCommandList);
        break;
    case JigAbstractInterface::plugin:
        this->commandsList->setStringList(pluginCommandList);
        break;
    default:
        this->commandsList->setStringList(appCommandList);
        break;
    }
}
