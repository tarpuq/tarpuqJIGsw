#ifndef DUTINFOFORM_H
#define DUTINFOFORM_H

#include <QWidget>

namespace Ui {
class Dut;
}

class Dut: public QWidget
{
    Q_OBJECT
    Q_ENUMS(DutStateType)

public:
    explicit Dut(QWidget *parent = nullptr);
    ~Dut();

    enum DutStateType{
        idle,
        wait,
        ready,
        repeated,
        jump,
        pass,
        nopass,
        undefined
    };

    void clearSerialNumber();

    quint64 getSerialNumber() const;
    void setSerialNumber(const quint64 &value);

    int getIndex() const;
    void setIndex(int value);

    DutStateType getStatus() const;
    void setStatus(DutStateType state);

    void clearRegister();
    void putRegister(QString error, QString comment, QString measure);

    bool getError() const;
    void setError(bool value);

    QStringList getFails() const;
    void setFails(const QStringList &value);

    QStringList getComments() const;
    void setComments(const QStringList &value);

    QStringList getMeasures() const;
    void setMeasures(const QStringList &value);

    bool getActivated() const;
    void setActivated(bool value);

public slots:
    void setFocusOnSerialNumber();

signals:
    void statusChanged(DutStateType state);
    void jumpSerial(int index);

private slots:
    void on_lineEdit_serialNumber_textChanged(const QString &arg1);

    void on_lineEdit_serialNumber_returnPressed();

    bool validateSerialNumber();

private:
    Ui::Dut *ui;
    QStringList fails;
    QStringList comments;
    QStringList measures;

    quint64 serialNumber;
    bool error;
    bool activated;
    DutStateType currentStatus;
    DutStateType lastStatus;
    int index;
};

#endif // DUTINFOFORM_H
