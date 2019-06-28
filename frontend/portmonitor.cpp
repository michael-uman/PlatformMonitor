#include <QSerialPort>
#include <QSerialPortInfo>
#include <QList>
#include <QDebug>
#include <QJsonDocument>
#include "portmonitor.h"

#if 0
static void enumerate_serial_ports()
{
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();

    for (auto serport : ports) {
        qDebug() << "Serial port : " << serport.portName();
    }
}
#endif

PortMonitor::PortMonitor(QObject *parent) : QObject(parent)
{
    serial_port = new QSerialPort("COM5");
    serial_port->setBaudRate(115200);
    serial_port->setDataBits(QSerialPort::Data8);
    serial_port->setStopBits(QSerialPort::OneStop);
    serial_port->setParity(QSerialPort::NoParity);
    if (serial_port->open(QIODevice::ReadWrite) == true) {
        qDebug() << "Serial port is open!";
    } else {
        qDebug() << "Serial port failed to open!";
    }

    connect(serial_port, SIGNAL(readyRead()), this, SLOT(onDataReady()));
}

void PortMonitor::onDataReady()
{
    QByteArray data;

    while (serial_port->canReadLine()) {
        data = serial_port->readLine(0);
        data = data.trimmed();
        parseJSON(data);
    }
}

void PortMonitor::parseJSON(QByteArray &buffer)
{
    QJsonDocument doc = QJsonDocument::fromJson(buffer);
    if (doc.isNull()) {
        qDebug() << "passed a null json";
        return;
    }
    emit msgReceived(doc);
}
