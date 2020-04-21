#include "jigttyinterface.h"

#include <QDebug>
#include <QSerialPortInfo>

JigTtyInterface::JigTtyInterface()
{
    sPort = new QSerialPort(this);

    debug = 0;

    connect(sPort, &QSerialPort::readyRead, this, &JigTtyInterface::readData);
}

int JigTtyInterface::open()
{
    int ans = 0;

    sPort->setPortName("");
    rxBuffer.clear();
    dataOk = false;

    if(!debug){
        qDebug() << QString("Buscando dispositivo con vid=%1, pid=%2").arg(QString::number(this->vid,16)).arg(QString::number(this->pid,16));
        debug = 1;
    }

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        if ((info.vendorIdentifier() == this->vid) && (info.productIdentifier() == this->pid)) {
            qDebug() << "Dispositivo encontrado en el puerto " << info.portName();
            sPort->setPortName(info.portName());
            debug = 0;
        }
    }

    if (!sPort->portName().isEmpty()) {
        qDebug() << "Opening" << sPort->portName() << sPort->baudRate() << sPort->dataBits()
                 << sPort->parity() << sPort->stopBits();

        if (sPort->open(QIODevice::ReadWrite)) {
            ans = 0;
        } else {
            ans = -1;
        }
    } else {
        ans = -2;
    }

    return ans;
}

void JigTtyInterface::close()
{
    qDebug() << "Closing" << sPort->portName();
    debug = 0;
    sPort->close();
}

void JigTtyInterface::config()
{
    this->vid = parameters["vid"].toString().toInt(nullptr,16);
    this->pid = parameters["pid"].toString().toInt(nullptr,16);

    if (parameters["portName"].toString().isEmpty()) {
        this->isVidPid = true;
        parameters["isVidPid"] = true;
    }

    if (!this->isVidPid)
        sPort->setPortName(parameters["portName"].toString());

    sPort->setBaudRate(static_cast<qint32>(parameters["baudRate"].toInt()));
    sPort->setDataBits(static_cast<QSerialPort::DataBits>(parameters["dataBits"].toInt()));
    sPort->setParity(static_cast<QSerialPort::Parity>(parameters["parity"].toInt()));
    sPort->setStopBits(static_cast<QSerialPort::StopBits>(parameters["stopBits"].toInt()));
}

bool JigTtyInterface::isOpen()
{
    return sPort->isOpen();
}

qint64 JigTtyInterface::write(const QByteArray &data)
{
    qDebug() << "TX:" << data.toHex('-') << " to: " << this->getAlias();
    sPort->readAll();
    sPort->flush();
    return sPort->write(data);
}

void JigTtyInterface::reset()
{
    rxBuffer.clear();
    dataOk = false;
}

int JigTtyInterface::getResult()
{
    return 0;
}

void JigTtyInterface::readData()
{
    while (sPort->canReadLine()) {
        QByteArray data = sPort->readLine();
        qDebug() << "rawRX: " << data << " from: " << this->getAlias();
        rxBuffer.append(data);

        int index = rxBuffer.indexOf(static_cast<char>(0));
        if (index >= 0) {
            rxBuffer.remove(index, 1);
        }
        dataOk = true;
    }
}
