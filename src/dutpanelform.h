#ifndef DutPanelForm_H
#define DutPanelForm_H

#include <QWidget>
#include "dut.h"

namespace Ui {
class DutPanelForm;
}

class DutPanelForm : public QWidget
{
    Q_OBJECT

public:
    explicit DutPanelForm(QWidget *parent = nullptr);
    ~DutPanelForm();

    QList<Dut*> *createDutMatrix(int rows, int cols);

public slots:
    bool isReady();
    void checkSerialNumbers(Dut::DutStateType state);
    void jumpSerialNumber(int index);
    void jumpFirstSerialNumber();
    QList<QString> getSerialNumbersList();
    void setDutStatus(int index, Dut::DutStateType state);
    QString getSerialNumber(int index);

    QList<bool> activatedDuts();

    void cleanUI();

signals:
    void dutsReady();

private:
    Ui::DutPanelForm *ui;
    QList<Dut*> _dutList;
    bool ready;
};

#endif // DutPanelForm_H
