#ifndef MESSAGESFORM_H
#define MESSAGESFORM_H

#include <QWidget>

namespace Ui {
class MessagesForm;
}

class MessagesForm : public QWidget
{
    Q_OBJECT

public:
    explicit MessagesForm(QWidget *parent = nullptr);
    ~MessagesForm();

public slots:
    void setOutputMessage(QString msg);
    void setDebugMessage(QString msg);

    void clearOutputMessage();
    void clearDebugMessage();

private:
    Ui::MessagesForm *ui;
};

#endif // MESSAGESFORM_H
