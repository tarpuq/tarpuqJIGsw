#ifndef DEBUGCONTROLUI_H
#define DEBUGCONTROLUI_H

#include <QWidget>
#include "dutpanelform.h"
#include "dut.h"

namespace Ui {
class DebugControlUi;
}

class DebugControlUi : public QWidget
{
    Q_OBJECT

public:
    explicit DebugControlUi(QWidget *parent = nullptr);
    ~DebugControlUi();

    void setDutList(QList<Dut *> *dutList);

public slots:
    void finished(int status);

private slots:
    void on_pushButton_clicked();

    void on_pushButton_Start_clicked();

    void on_pushButton_Stop_clicked();

    void on_actionStart_triggered();

signals:
    void start();

private:
    Ui::DebugControlUi *ui;
    QList<Dut *> *_dutList;
    quint64 currentSerial;
    int currentIteration;
    int currentErrors;
    bool running;
};

#endif // DEBUGCONTROLUI_H
