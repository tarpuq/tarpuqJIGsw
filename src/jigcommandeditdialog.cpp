#include "jigcommandeditdialog.h"
#include "ui_jigcommandeditdialog.h"

#include <QDebug>

JigCommandEditDialog::JigCommandEditDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::JigCommandEditDialog)
{
    ui->setupUi(this);

    interfacesList = new QStringListModel(this);

    commandsList = new QStringListModel(this);

    ui->comboBox_tool->setModel(interfacesList);

    ui->comboBox_command->setModel(commandsList);
}

JigCommandEditDialog::~JigCommandEditDialog()
{
    delete ui;
}

void JigCommandEditDialog::loadCommand(int index, JigSyncCommand *cmd)
{
    this->index = index;
    this->command = cmd;

    ui->comboBox_tool->setCurrentText(command->getInterfaceName());
    ui->comboBox_command->setCurrentText(command->getInterfaceCommand());
    ui->lineEdit_ifArgs->setText(command->getInterfaceArguments());
    ui->lineEdit_ifAns->setText(command->getInterfaceAnswer());
    ui->doubleSpinBox_deviation->setValue(command->deviation);
    ui->doubleSpinBox_meanFixed->setValue(command->mean);
    ui->doubleSpinBox_offset->setValue(command->offset);
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
    ui->checkBox_meanFormula->setChecked(command->getUseMeanFormula());
    ui->lineEdit_meanFormula->setText(command->getMeanFormula());
    ui->lineEdit_meanFormula->setEnabled(command->getUseMeanFormula());
    ui->lineEdit_measFormula->setText(command->getMeasFormula());

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
}

void JigCommandEditDialog::on_buttonBox_accepted()
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
    command->setMeasureParameters(ui->doubleSpinBox_meanFixed->value(),
                                  ui->doubleSpinBox_deviation->value(), ui->doubleSpinBox_offset->value());
    command->runOnJump = ui->checkBox_runOnJump->isChecked();
    command->setUseMeanFormula(ui->checkBox_meanFormula->isChecked());
    command->setMeanFormula(ui->lineEdit_meanFormula->text());
    command->setMeasFormula(ui->lineEdit_measFormula->text());

    //      USART
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

    command->setState(JigSyncCommand::pending);
}

void JigCommandEditDialog::setInterfaces(QHash<QString, JigInterface *> *interfaces)
{
    this->interfaces = interfaces;
    interfacesList->setStringList(this->interfaces->keys());
}

void JigCommandEditDialog::setCommands(QStringList commands)
{
    otherCommandsList = commands;
}

void JigCommandEditDialog::setTtyCommandsList(const QStringList &value)
{
    ttyCommandsList = value;
}

void JigCommandEditDialog::setUsbCommandsList(const QStringList &value)
{
    usbCommandsList = value;
}

void JigCommandEditDialog::setPluginCommandsList(const QStringList &value)
{
    pluginCommandsList = value;
}

void JigCommandEditDialog::on_doubleSpinBox_meanFixed_valueChanged(double arg1)
{
    qDebug() << arg1;

    double max = arg1 + ui->doubleSpinBox_deviation->value();
    double min = arg1 - ui->doubleSpinBox_deviation->value();

    ui->lineEdit_measMax->setText(QString::number(max));
    ui->lineEdit_measMin->setText(QString::number(min));
}

void JigCommandEditDialog::on_doubleSpinBox_deviation_valueChanged(double arg1)
{
    qDebug() << arg1;

    double max = ui->doubleSpinBox_meanFixed->value() + arg1;
    double min = ui->doubleSpinBox_meanFixed->value() - arg1;

    ui->lineEdit_measMax->setText(QString::number(max));
    ui->lineEdit_measMin->setText(QString::number(min));
}

void JigCommandEditDialog::on_checkBox_meanFormula_toggled(bool checked)
{
    ui->lineEdit_meanFormula->setEnabled(checked);
    ui->doubleSpinBox_meanFixed->setEnabled(!checked);
}

void JigCommandEditDialog::on_comboBox_tool_currentIndexChanged(const QString &arg1)
{
    JigInterface::JigInterfaceType type;
    type = interfaces->value(arg1)->getType();

    switch (type) {
    case JigInterface::tty:
        this->commandsList->setStringList(ttyCommandsList);
        break;
    case JigInterface::usb:
        this->commandsList->setStringList(usbCommandsList);
        break;
    case JigInterface::plugin:
        this->commandsList->setStringList(pluginCommandsList);
        break;
    default:
        this->commandsList->setStringList(otherCommandsList);
        break;
    }
}
