#include "dut.h"
#include "ui_dut.h"

#include <QDebug>

Dut::Dut(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Dut)
{
    ui->setupUi(this);

    serialNumber = 0;

    activated = false;
    lastStatus = undefined;

    setStatus(idle);
}

Dut::~Dut()
{
    delete ui;
}

void Dut::clearSerialNumber()
{
    ui->lineEdit_serialNumber->blockSignals(true);
    ui->lineEdit_serialNumber->clear();
    ui->lineEdit_serialNumber->blockSignals(false);
    this->serialNumber = 0;
    this->activated = 0;
}

quint64 Dut::getSerialNumber() const
{
    return serialNumber;
}

void Dut::setSerialNumber(const quint64 &value)
{
    serialNumber = value;
    if(validateSerialNumber()){
        ui->lineEdit_serialNumber->setText(QString::number(serialNumber));
    }
}

void Dut::on_lineEdit_serialNumber_textChanged(const QString &arg1)
{
    serialNumber = static_cast<quint64>(arg1.toLongLong());
    validateSerialNumber();
}

int Dut::getIndex() const
{
    return index;
}

void Dut::setIndex(int value)
{
    index = value;

    QString title = QString("DUT %1").arg(index + 1);
    ui->groupBox->setTitle(title);
}

void Dut::setFocusOnSerialNumber()
{
    ui->lineEdit_serialNumber->setFocus();
}

void Dut::on_lineEdit_serialNumber_returnPressed()
{
    emit jumpSerial(index);
}

bool Dut::validateSerialNumber()
{
    QString serialStr = QString::number(this->serialNumber);

    if ((this->serialNumber != 0) && (serialStr.size() == 11)){
        activated = true;
        setStatus(ready);
    } else {
        activated = false;
        this->serialNumber = 0;
        setStatus(idle);
    }

    return activated;
}

bool Dut::getActivated() const
{
    return activated;
}

void Dut::setActivated(bool value)
{
    activated = value;
}

QStringList Dut::getMeasures() const
{
    return measures;
}

void Dut::setMeasures(const QStringList &value)
{
    measures = value;
}

QStringList Dut::getComments() const
{
    return comments;
}

void Dut::setComments(const QStringList &value)
{
    comments = value;
}

QStringList Dut::getFails() const
{
    return fails;
}

void Dut::setFails(const QStringList &value)
{
    fails = value;
}

bool Dut::getError() const
{
    return error;
}

void Dut::setError(bool value)
{
    error = value;
}

Dut::DutStateType Dut::getStatus() const
{
    return currentStatus;
}

void Dut::setStatus(DutStateType state)
{
    if(lastStatus == state)
        return;

    currentStatus = state;

    switch (state) {
        case idle:
            ui->label_status->setText("INACTIVO");
            ui->label_status->setStyleSheet("background-color: normal;");
            break;
        case wait:
            ui->label_status->setText("ESPERE");
            ui->label_status->setStyleSheet("background-color: normal;");
            break;
        case repeated:
            ui->label_status->setText("REPETIDO");
            ui->label_status->setStyleSheet("background-color: red;");
            break;
        case ready:
            ui->label_status->setText("LISTO");
            ui->label_status->setStyleSheet("background-color: normal;");
            break;
        case jump:
            ui->label_status->setText("SALTA");
            ui->label_status->setStyleSheet("background-color: normal;");
            break;
        case pass:
            ui->label_status->setText("PASA");
            ui->label_status->setStyleSheet("background-color: green;");
            break;
        case nopass:
            ui->label_status->setText("NO PASA");
            ui->label_status->setStyleSheet("background-color: red;");
            break;
        case undefined:
            break;
    }

    emit statusChanged(currentStatus);

    lastStatus = currentStatus;
}

void Dut::clearRegister()
{
    this->fails.clear();
    this->comments.clear();
    this->measures.clear();
}

void Dut::putRegister(QString fail, QString comment, QString measure)
{
    this->fails << fail;
    this->comments << comment;
    this->measures << measure;
}
