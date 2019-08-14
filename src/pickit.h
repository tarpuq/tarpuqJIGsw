#ifndef PICKIT_H
#define PICKIT_H

#include <QProcess>

class Pickit : public QProcess
{
    Q_OBJECT
public:
    explicit Pickit(QObject *parent = nullptr);

    QString lastError;

    bool isBatched;

    enum ipeStates {
        Off,
        Starting,
        Idle,
        Connecting,
        TargetVoltageDetect,
        TargetFound,
        TargetRemoved,
        Programming,
        ProgrammingComplete,
        Success,
        TargetNotFound,
        Fail,
        Ready
    };
    Q_ENUM(ipeStates)

    bool runPickit(void);
    void restart(void);

    ipeStates state(void) { return this->prvState; }
    QString ipePath(void) { return this->prvIpePath; }
    QString hexPath(void) { return this->prvHexPath; }
    QString pickitSerialNumber(void) { return this->prvPickitSerialNumber; }
    QString targetName(void) { return this->prvTargetName; }
    QString mclrPinName(void) { return this->prvMclrPinName; }
    QString vccPinName(void) { return this->prvVccPinName; }
    QString gndPinName(void) { return this->prvGndPinName; }
    QString pgdPinName(void) { return this->prvPgdPinName; }
    QString pgcPinName(void) { return this->prvPgcPinName; }

signals:
    void ipeOutput(QString);
    void ipeMessage();
    void ipeWarning();
    void ipeError();

public slots:
    void setIfArguments(const QString &value, const QHash<QString, QByteArray> &wildcard);
    void setIpePath(const QString name) { this->prvIpePath = name; }
    void setHexPath(const QString name) { this->prvHexPath = name; }
    void setPickitSerialNumber(const QString name) { this->prvPickitSerialNumber = name; }
    void setTargetName(const QString name) { this->prvTargetName = name; }
    void setMclrPinName(const QString name) { this->prvMclrPinName = name; }
    void setVccPinName(const QString name) { this->prvVccPinName = name; }
    void setGndPinName(const QString name) { this->prvGndPinName = name; }
    void setPgdPinName(const QString name) { this->prvPgdPinName = name; }
    void setPgcPinName(const QString name) { this->prvPgcPinName = name; }
    void ipeStdout();
    void ipeStderr();

private:
    ipeStates prvState;
    QStringList prvIpeArgs;
    QString prvIpePath;
    QString prvHexPath;
    QString prvPickitSerialNumber;
    QString prvTargetName;
    QString prvMclrPinName;
    QString prvVccPinName;
    QString prvGndPinName;
    QString prvPgdPinName;
    QString prvPgcPinName;
};

#endif // PICKIT_H
