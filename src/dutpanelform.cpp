#include "dutpanelform.h"
#include "ui_dutpanelform.h"

#include <QMdiSubWindow>

#include <QDebug>

DutPanelForm::DutPanelForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DutPanelForm)
{
    ui->setupUi(this);
}

DutPanelForm::~DutPanelForm()
{
    delete ui;
}

QList<Dut*> *DutPanelForm::createDutMatrix(int rows, int cols)
{
    if (_dutList.size() > 0){
        foreach(Dut *dut, _dutList){
            ui->gridLayout->removeWidget(dut);
            dut->deleteLater();
        }
        _dutList.clear();
    }

    for (int i=0; i< rows; i++) {
        for (int j = 0; j < cols; j++) {
            Dut *dut = new Dut(this);

            dut->setIndex(cols * i +j);

            connect(dut, &Dut::statusChanged, this, &DutPanelForm::checkSerialNumbers);
            connect(dut, &Dut::jumpSerial, this, &DutPanelForm::jumpSerialNumber);

            ui->gridLayout->addWidget(dut,i,j);

            _dutList << dut;
        }
    }

    static_cast<QMdiSubWindow *>(this->parent())->resize(480, 240);

    return &_dutList;
}

bool DutPanelForm::isReady()
{
    return ready;
}

void DutPanelForm::checkSerialNumbers(Dut::DutStateType state)
{
    if((state == Dut::idle) || (state == Dut::ready)){
        QList<quint64> sNumbers;
        QList<int> repeatedIndex;
        int i = 0;
        ready = true;

        //  Set ready to all
        for (i = 0; i < _dutList.size(); i++) {
            Dut *dut = _dutList.at(i);
            dut->blockSignals(true);
            if (dut->getActivated()){
                dut->setStatus(Dut::ready);
            }
            dut->blockSignals(false);
        }

        //  Check repeated
        for (i = 0; i < _dutList.size() - 1; i++) {
            Dut *dut = _dutList.at(i);
            dut->blockSignals(true);
            quint64 sNumber = dut->getSerialNumber();

            if (sNumber != 0){
                for (int j = i + 1; j < _dutList.size(); j++) {
                    Dut *cDut = _dutList.at(j);
                    quint64 cNumber = cDut->getSerialNumber();

                    if (cNumber != 0){
                        if (dut->getSerialNumber() == cDut->getSerialNumber()){
                            dut->setStatus(Dut::repeated);
                            cDut->setStatus(Dut::repeated);
                            ready = false;
                        }
                    }
                }
            }

            dut->blockSignals(false);
        }
        emit dutsReady();
    }
}

void DutPanelForm::jumpSerialNumber(int index)
{
    if((index + 1) < _dutList.size()){
        _dutList.at(index + 1)->setFocusOnSerialNumber();
    }
}

void DutPanelForm::jumpFirstSerialNumber()
{
    _dutList.at(0)->setFocusOnSerialNumber();
}

QList<QString> DutPanelForm::getSerialNumbersList()
{
    QList<QString> list;

    foreach(Dut *dut, _dutList){
        list << QString::number(dut->getSerialNumber());
    }
    return  list;
}

void DutPanelForm::setDutStatus(int index, Dut::DutStateType state)
{
    if(index < _dutList.size())
        _dutList.at(index)->setStatus(state);
}

QString DutPanelForm::getSerialNumber(int index)
{
    return QString::number(_dutList.at(index)->getSerialNumber());
}

QList<bool> DutPanelForm::activatedDuts()
{
    QList<bool> jump;

    foreach(Dut *dut, _dutList){
        jump << !dut->getActivated();
    }

    return jump;
}

void DutPanelForm::cleanUI()
{
    foreach(Dut *dut, _dutList){
        dut->setStatus(Dut::idle);
        dut->clearRegister();
        dut->setError(false);
    }
}
