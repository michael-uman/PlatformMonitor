#include <QSerialPort>
#include <QSerialPortInfo>
#include <QList>
#include <QDebug>
#include <QJsonDocument>
#include "portmonitor.h"
#include "commands.h"

#if defined(Q_OS_WIN32)
    #define COMPORT "COM4"
#elif defined(Q_OS_LINUX)
    #define COMPORT "/dev/ttyACM0"
#endif

/**
 * @brief   The port monitor class interfaces with the device through
 *          the UART.
 * @param parent
 */
PortMonitor::PortMonitor(QObject *parent) : QObject(parent)
{
    serial_port = new QSerialPort(COMPORT);
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

/**
 * @brief Parse the JSON buffer and pass it to all slots connected to the signal.
 * @param buffer
 */
void PortMonitor::parseJSON(QByteArray &buffer)
{
    QJsonDocument doc = QJsonDocument::fromJson(buffer);
    if (doc.isNull()) {
        qDebug() << "passed a null json : " << buffer;
        return;
    }
    emit msgReceived(doc);
}

/**
 * @brief Display all available serial ports...
 */
void PortMonitor::DebugSerialPorts()
{
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();

    for (auto serport : ports) {
        qDebug() << "Serial port : " << serport.portName();
    }
}

/**
 * @brief Send the command to turn a LED on.
 * @param led - LED # to turn on
 */
void PortMonitor::TurnLEDOn(uint32_t led)
{
    qDebug() << Q_FUNC_INFO;

    if (serial_port) {
        RECVCMD_t   cmd = {
            RECVCMD_LEDON,
            led
        };
        serial_port->write(reinterpret_cast<const char*>(&cmd), sizeof(cmd));
        serial_port->flush();
    }
}

/**
 * @brief Send the command to turn a LED off.
 * @param led - LED # to turn off
 */
void PortMonitor::TurnLEDOff(uint32_t led)
{
    qDebug() << Q_FUNC_INFO;

    if (serial_port) {
        RECVCMD_t   cmd = {
            RECVCMD_LEDOFF,
            led
        };
        serial_port->write(reinterpret_cast<const char*>(&cmd), sizeof(cmd));
        serial_port->flush();
    }
}

void PortMonitor::GetVersion()
{
    qDebug() << Q_FUNC_INFO;

    if (serial_port) {
        RECVCMD_t   cmd = {
            RECVCMD_VERSION,
            0
        };
        serial_port->write(reinterpret_cast<const char*>(&cmd), sizeof(cmd));
        serial_port->flush();
    }
}
