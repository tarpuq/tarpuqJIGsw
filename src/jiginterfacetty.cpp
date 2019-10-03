#include "jiginterfacetty.h"

#include <QDebug>
#include <QSerialPortInfo>

JigInterfaceTty::JigInterfaceTty()
{
    sPort = new QSerialPort(this);

    connect(sPort, &QSerialPort::readyRead, this, &JigInterfaceTty::readData);
}

int JigInterfaceTty::open()
{
    int ans = 0;

    sPort->setPortName("");

    qDebug() << QString("Buscando dispositivo con vid=%1, pid=%2").arg(QString::number(this->vid,16)).arg(QString::number(this->pid,16));
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        if ((info.vendorIdentifier() == this->vid) && (info.productIdentifier() == this->pid)) {
            qDebug() << "Encontrado el puerto en" << info.portName();
            sPort->setPortName(info.portName());
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

void JigInterfaceTty::close()
{
    sPort->close();
}

void JigInterfaceTty::config()
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

bool JigInterfaceTty::isOpen()
{
    return sPort->isOpen();
}

qint64 JigInterfaceTty::write(const QByteArray &data)
{
    qDebug() << "TX:" << data.toHex('-');
    sPort->readAll();
    sPort->flush();
    return sPort->write(data);
}

void JigInterfaceTty::readData()
{
    while (sPort->canReadLine()) {
        QByteArray data = sPort->readLine();
        qDebug() << "rawRX: " << data;
        rxBuffer.append(data);

        int index = rxBuffer.indexOf(static_cast<char>(0));
        if (index >= 0) {
            rxBuffer.remove(index, 1);
        }
        dataOk = true;
    }
}
