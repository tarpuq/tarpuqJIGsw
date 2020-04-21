#include "debugcontrolui.h"
#include "ui_debugcontrolui.h"

#include <QDebug>

DebugControlUi::DebugControlUi(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DebugControlUi)
{
    ui->setupUi(this);

    _dutList = nullptr;
}

DebugControlUi::~DebugControlUi()
{
    delete ui;
}

void DebugControlUi::on_pushButton_clicked()
{
    if(_dutList){
        foreach(Dut *dut, *_dutList){
            Dut::DutStateType state = static_cast<Dut::DutStateType>(ui->comboBox->currentIndex());
            dut->setStatus(state);
        }
    }
}

void DebugControlUi::setDutList(QList<Dut *> *dutList)
{
    _dutList = dutList;
}

void DebugControlUi::finished(int status)
{
    if(running){
        currentIteration++;

        currentErrors += status;

        ui->lineEdit_ErrorsCounter->setText(QString::number(currentErrors));

        if(currentIteration < ui->spinBox_Iterations->value()){
            ui->actionStart->trigger();
            qDebug() << "restart";
        }
    }
}

void DebugControlUi::on_pushButton_Start_clicked()
{
    currentSerial = static_cast<quint64>(ui->lineEdit->text().toLongLong());
    currentIteration = 0;
    currentErrors = 0;
    running = true;

    if(_dutList){
        ui->actionStart->trigger();
    } else {
        running = false;
    }
}

void DebugControlUi::on_pushButton_Stop_clicked()
{
    running = false;
}

void DebugControlUi::on_actionStart_triggered()
{
    foreach(Dut *dut, *_dutList){
        dut->setSerialNumber(currentSerial);
        if(ui->checkBox_IncrementalSerial->isChecked()){
            currentSerial++;
        }
    }

    ui->lineEdit_currentIteration->setText(QString::number(currentIteration + 1));

    emit start();
}
