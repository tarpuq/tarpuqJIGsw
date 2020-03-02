#ifndef DUTSTATUSFORM_H
#define DUTSTATUSFORM_H

#include <QWidget>

namespace Ui {
class DutInfoForm;
}

class DutInfoForm : public QWidget
{
    Q_OBJECT

public:
    explicit DutInfoForm(QWidget *parent = nullptr);
    ~DutInfoForm();

    void clearSerialNumber();

    quint64 getSerialNumber() const;
    void setSerialNumber(const quint64 &value);

    int getIndex() const;
    void setIndex(int value);

    int getStatus() const;
    void setStatus(int value);

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

public slots:
    void setFocusOnSerialNumber();

signals:
    void serialNumberUpdated();
    void jumpSerial(int index);

private slots:
    void on_lineEdit_serialNumber_textChanged(const QString &arg1);

    void on_lineEdit_serialNumber_returnPressed();

private:
    Ui::DutInfoForm *ui;
    QStringList fails;
    QStringList comments;
    QStringList measures;

    quint64 serialNumber;
    bool serialNumberReady;
    bool error;
    int status;
    int index;
};

#endif // DUTSTATUSFORM_H
