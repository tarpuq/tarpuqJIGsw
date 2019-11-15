#ifndef CONSOLE_H
#define CONSOLE_H

#include <QDialog>

namespace Ui {
class Console;
}

class Console : public QDialog
{
    Q_OBJECT

public:
    explicit Console(QWidget *parent = nullptr);
    ~Console();

private slots:
    void on_pushButton_clicked();

private:
    Ui::Console *ui;
};

#endif // CONSOLE_H
