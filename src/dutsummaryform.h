#ifndef DUTSUMMARYFORM_H
#define DUTSUMMARYFORM_H

#include <QWidget>
#include "dutinfoform.h"

namespace Ui {
class DutSummaryForm;
}

class DutSummaryForm : public QWidget
{
    Q_OBJECT

public:
    explicit DutSummaryForm(QWidget *parent = nullptr);
    ~DutSummaryForm();

    QList<DutInfoForm*> *createDutMatrix(int rows, int cols);
    void clearDutSN();

public slots:
    bool isReady();
    void checkSerialNumbers();
    void jumpSerialNumber(int index);
    QList<QString> getSerialNumbersList();
    void setDutStatus(int index, int status);
    QString getSerialNumber(int index);

    void cleanUI();

signals:
    void dutsReady();
    void dutChanged(bool state);

private:
    Ui::DutSummaryForm *ui;
    QList<DutInfoForm *> _dutList;
    bool ready;
};

#endif // DUTSUMMARYFORM_H
