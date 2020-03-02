#include "dutsummaryform.h"
#include "ui_dutsummaryform.h"

#include <QMdiSubWindow>

#include <QDebug>

DutSummaryForm::DutSummaryForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DutSummaryForm)
{
    ui->setupUi(this);
}

DutSummaryForm::~DutSummaryForm()
{
    delete ui;
}

QList<DutInfoForm *> *DutSummaryForm::createDutMatrix(int rows, int cols)
{
    if (_dutList.size() > 0){
        foreach(DutInfoForm *w, _dutList){
            ui->gridLayout->removeWidget(w);
            w->deleteLater();
        }
        _dutList.clear();
    }

    for (int i=0; i< rows; i++) {
        for (int j = 0; j < cols; j++) {
            DutInfoForm *dutInfo = new DutInfoForm(this);

            dutInfo->setIndex(cols * i +j);

            connect(dutInfo,&DutInfoForm::serialNumberUpdated,this, &DutSummaryForm::checkSerialNumbers);
            connect(dutInfo,&DutInfoForm::jumpSerial,this,&DutSummaryForm::jumpSerialNumber);

            ui->gridLayout->addWidget(dutInfo,i,j);

            _dutList << dutInfo;
        }
    }

    static_cast<QMdiSubWindow *>(this->parent())->resize(480, 240);

    return &_dutList;
}

void DutSummaryForm::clearDutSN()
{
    foreach(DutInfoForm *w, _dutList){
        w->clearSerialNumber();
    }
    ready = false;
}

bool DutSummaryForm::isReady()
{
    return ready;
}

void DutSummaryForm::checkSerialNumbers()
{
    ready = true;
    foreach(DutInfoForm *w, _dutList){
        if(w->getSerialNumber() == 0){
            ready = false;
        }
    }
    emit dutChanged(ready);
}

void DutSummaryForm::jumpSerialNumber(int index)
{
    if((index + 1) < _dutList.size()){
        _dutList.at(index + 1)->setFocusOnSerialNumber();
    } else {
        emit dutsReady();
    }
}

QList<QString> DutSummaryForm::getSerialNumbersList()
{
    QList<QString> list;

    foreach(DutInfoForm *w, _dutList){
        list << QString::number(w->getSerialNumber());
    }
    return  list;
}

void DutSummaryForm::setDutStatus(int index, int status)
{
    if(index < _dutList.size())
        _dutList.at(index)->setStatus(status);
}

QString DutSummaryForm::getSerialNumber(int index)
{
    return QString::number(_dutList.at(index)->getSerialNumber());
}

void DutSummaryForm::cleanUI()
{
    foreach(DutInfoForm *w, _dutList){
        w->setStatus(0);
        w->clearRegister();
        w->setError(false);
    }
}
