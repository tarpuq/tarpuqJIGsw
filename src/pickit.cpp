#include "pickit.h"

#include <QDebug>
#include <QObject>

#include <QtGlobal>

Pickit::Pickit(QObject *parent)
    : QProcess(parent)
{
    connect(this, &Pickit::readyReadStandardOutput, this, &Pickit::ipeStdout);
    connect(this, &Pickit::readyReadStandardError, this, &Pickit::ipeStderr);
    connect(this, qOverload<int, QProcess::ExitStatus>(&Pickit::finished), [this]() {
        this->prvState = Off;
    });
    connect(this, &Pickit::started, [this]() { this->prvState = Starting; });

    prvState = Off;
}

bool Pickit::runPickit()
{
    QString target = prvTargetName.mid(3);

    prvIpeArgs.clear();
    prvIpeArgs << QString("-TS").append(prvPickitSerialNumber);
    prvIpeArgs << QString("-P").append(target);
    prvIpeArgs << QString("-F").append(prvHexPath);
    prvIpeArgs << "-M";
    prvIpeArgs << "-OL";
    prvIpeArgs << "-OB";

    qDebug() << "ipeCmd" << prvIpePath << prvIpeArgs;

    if (prvState == Off) {
        setProgram(prvIpePath);
        setArguments(prvIpeArgs);
        qDebug() << program() << arguments();
        start(QIODevice::ReadWrite);
        return true;
    } else if (isBatched) {
        restart();
        return true;
    } else {
        return false;
    }
}

void Pickit::restart()
{
    prvState = Starting;
    this->write("\n");
}

void Pickit::setIfArguments(const QString &value, const QHash<QString, QByteArray> &wildcard)
{

}

void Pickit::ipeStdout()
{
    QString stdoutStr;
    stdoutStr = readAllStandardOutput();

    if (!stdoutStr.isEmpty()) {
        emit ipeOutput(stdoutStr);
    }

    if (stdoutStr.contains("******")) {
        prvState = Idle;
    }
    if (stdoutStr.contains("Connecting to MPLAB PICkit 3")) {
        prvState = Connecting;
        isBatched = false;
    }
    if (stdoutStr.contains("Currently loaded firmware on PICkit 3")) {
    }
    if (stdoutStr.contains("Firmware Suite Version.....")) {
    }
    if (stdoutStr.contains("Firmware type.....")) {
    }
    if (stdoutStr.contains("Target voltage detected")) {
        prvState = TargetVoltageDetect;
    }
    if (stdoutStr.contains("Target device " + prvTargetName + " found")) {
        prvState = TargetFound;
        emit ipeMessage();
    }
    if (stdoutStr.contains("Device Revision ID = ")) {
    }
    if (stdoutStr.contains("Device Erased...")) {
    }
    if (stdoutStr.contains("The following memory area(s) will be programmed:")) {
    }
    if (stdoutStr.contains("program memory: start address = 0x0, end address = 0xf9ff")) {
    }
    if (stdoutStr.contains("configuration memory")) {
    }
    if (stdoutStr.contains("Programming...")) {
        prvState = Programming;
    }
    if (stdoutStr.contains("Programming/Verify complete")) {
        prvState = ProgrammingComplete;
    }
    if (stdoutStr.contains("Target removed")) {
        prvState = TargetRemoved;
        emit ipeMessage();
    }
    if (stdoutStr.contains("Program Succeeded")) {
        prvState = Success;
    }
    if (stdoutStr.contains("Batch Mode Count")) {
    }
    if (stdoutStr.contains("Batch Mode:")) {
        isBatched = true;
    }
    if (stdoutStr.contains("Failed to get Device ID")) {
        prvState = TargetNotFound;
        lastError = "Error en la conexión con el picKit, revisar los pines " + prvMclrPinName + ", "
                    + prvPgcPinName + " y " + prvPgdPinName;
    }
    if (stdoutStr.contains("Target Device ID (0x0) is an Invalid Device ID.")) {
        prvState = TargetNotFound;
        lastError = "Error en la conexión con el picKit, revisar los pines " + prvMclrPinName + ", "
                    + prvPgcPinName + " y " + prvPgdPinName;
    }
    if (stdoutStr.contains("Target device was not found (could not detect target voltage VDD)")) {
        prvState = TargetNotFound;
        lastError = "No se ha detectado el dispositivo, posible conexión defectuosa en los pines "
                    + prvVccPinName + " y " + prvGndPinName + ", revisar conexiones.";
    }
    if (stdoutStr.contains("Target Vdd not detected.")) {
        prvState = TargetNotFound;
        lastError = "No se ha detectado el dispositivo, posible conexión defectuosa en los pines "
                    + prvVccPinName + " y " + prvGndPinName + ", revisar conexiones.";
    }
    if (stdoutStr.contains("Invalid Serial No.")) {
        prvState = Fail;
        lastError = "El pickit con el número de serie " + prvPickitSerialNumber
                    + " no fue encontrado, revise las conexiones y vuelva a intentar, si el "
                      "problema persiste reinicie el PC.";
    }
    if (stdoutStr.contains("Hex File could not be read")) {
        prvState = Fail;
        lastError = "El archivo Hex con el firmware no fue encontrado";
    }
    if (stdoutStr.contains("Could not find device")) {
        prvState = Fail;
        lastError = "No se puede encontrar el dispositivo " + prvTargetName;
    }
    if (stdoutStr.contains("Connection Failed")) {
        prvState = Fail;
        lastError = "Error en la conexión con el picKit.";
    }
    if (stdoutStr.contains("Programming Target Failed")) {
        prvState = Fail;
        lastError = "Error al programar el dispositivo.";
    }

    if (prvState == TargetNotFound) {
        emit ipeWarning();
    }

    if (prvState == Fail) {
        emit ipeError();
    }
}

void Pickit::ipeStderr()
{
    QString stderrStr;
    stderrStr = readAllStandardError();

    if (!stderrStr.isEmpty()) {
        qDebug() << stderrStr;

        lastError = stderrStr;

        emit ipeError();

        prvState = Fail;
    }
}
