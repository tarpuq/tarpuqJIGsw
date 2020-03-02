#include "dutinfoform.h"
#include "ui_dutinfoform.h"

#include <QDebug>

DutInfoForm::DutInfoForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DutInfoForm)
{
    ui->setupUi(this);

    serialNumber = 0;
    serialNumberReady = false;

    status = 0;
    setStatus(status);
}

DutInfoForm::~DutInfoForm()
{
    delete ui;
}

void DutInfoForm::clearSerialNumber()
{
    ui->lineEdit_serialNumber->clear();
}

quint64 DutInfoForm::getSerialNumber() const
{
    return serialNumber;
}

void DutInfoForm::setSerialNumber(const quint64 &value)
{
    serialNumber = value;
}

void DutInfoForm::on_lineEdit_serialNumber_textChanged(const QString &arg1)
{
    quint64 tempSerialNumber = static_cast<quint64>(arg1.toLongLong());
    bool aux = serialNumberReady;

    if(tempSerialNumber != 0){
        serialNumber = static_cast<quint64>(tempSerialNumber);

        if ((serialNumber != 0) && (arg1.size() == 11)){
            serialNumberReady = true;
        } else {
            serialNumberReady = false;
            serialNumber = 0;
        }
    } else {
        serialNumberReady = false;
        serialNumber = 0;
    }

    if(aux != serialNumberReady){
        emit serialNumberUpdated();
    }
}

int DutInfoForm::getIndex() const
{
    return index;
}

void DutInfoForm::setIndex(int value)
{
    index = value;

    QString title = QString("DUT %1").arg(index + 1);
    ui->groupBox->setTitle(title);
}

void DutInfoForm::setFocusOnSerialNumber()
{
    ui->lineEdit_serialNumber->setFocus();
}

void DutInfoForm::on_lineEdit_serialNumber_returnPressed()
{
    emit jumpSerial(index);
}

QStringList DutInfoForm::getMeasures() const
{
    return measures;
}

void DutInfoForm::setMeasures(const QStringList &value)
{
    measures = value;
}

QStringList DutInfoForm::getComments() const
{
    return comments;
}

void DutInfoForm::setComments(const QStringList &value)
{
    comments = value;
}

QStringList DutInfoForm::getFails() const
{
    return fails;
}

void DutInfoForm::setFails(const QStringList &value)
{
    fails = value;
}

bool DutInfoForm::getError() const
{
    return error;
}

void DutInfoForm::setError(bool value)
{
    error = value;
}

int DutInfoForm::getStatus() const
{
    return status;
}

void DutInfoForm::setStatus(int value)
{
    status = value;

    switch (value) {
        case 0:
            ui->label_status->setText("ESPERE");
            ui->label_status->setStyleSheet("background-color: normal;");
            break;
        case 1:
            ui->label_status->setText("PASA");
            ui->label_status->setStyleSheet("background-color: green;");
            break;
        case 2:
            ui->label_status->setText("NO PASA");
            ui->label_status->setStyleSheet("background-color: red;");
            break;
    }
}

void DutInfoForm::clearRegister()
{
    this->fails.clear();
    this->comments.clear();
    this->measures.clear();
}

void DutInfoForm::putRegister(QString fail, QString comment, QString measure)
{
    this->fails << fail;
    this->comments << comment;
    this->measures << measure;
}
