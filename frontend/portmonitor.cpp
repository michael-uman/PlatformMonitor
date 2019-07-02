#include <QSerialPort>
#include <QSerialPortInfo>
#include <QList>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include "portmonitor.h"
#include "commands.h"
#include "bcdstuff.h"

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
        ok = true;
    } else {
        qDebug() << "Serial port failed to open!";
    }

    if (ok) {
        connect(serial_port, SIGNAL(readyRead()), this, SLOT(onDataReady()));
        // Wait 1/2 second and send a get version packet...
        QTimer::singleShot(1500, this, &PortMonitor::GetVersion);
    }
}

PortMonitor::~PortMonitor()
{
    serial_port->close();
    delete serial_port;
}

bool PortMonitor::isOk() const
{
    return ok;
}

/**
 * @brief Slot called when data is available to read from serial port
 */
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

    QJsonObject obj = doc.object();
    QString sText;

    if (obj.contains("version") && obj.contains("name")) {
        sText = obj["name"].toString() + " " + obj["version"].toString();
        if (sText != version) {
            version = sText;
            emit versionChanged(version);
        }
    } else if (obj.contains("message")) {
        deviceMsg = obj["message"].toString();
        emit deviceMsgChanged(deviceMsg);
    } else if (obj.contains("id") &&
               obj.contains("button") &&
               obj.contains("led") &&
               obj.contains("timestamp")) {
        int _id     = obj["id"].toInt();
        int _button = obj["button"].toInt();
        int _led    = obj["led"].toInt();
        QString _ts = obj["timestamp"].toString();

        if (_id != id) {
            id = _id;
            emit idChanged(id);
        }
        if (_button != button) {
            button = _button;
            emit buttonChanged(button);
        }
        if (_led != led) {
            led = _led;
            emit ledChanged(led);
        }
        if (_ts != timestamp) {
            timestamp = _ts;
            emit tsChanged(timestamp);
        }
    } else {
        qDebug() << "Looks like invalid JSON to me!";
    }

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
    uint32_t ledMask = (1 << led);
    qDebug() << Q_FUNC_INFO;

    if (serial_port) {
        RECVCMD_t   cmd = {
            RECVCMD_LEDON,
            ledMask
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
    uint32_t ledMask = (1 << led);

    qDebug() << Q_FUNC_INFO;

    if (serial_port) {
        RECVCMD_t   cmd = {
            RECVCMD_LEDOFF,
            ledMask
        };
        serial_port->write(reinterpret_cast<const char*>(&cmd), sizeof(cmd));
        serial_port->flush();
    }
}

void PortMonitor::TurnAllLEDsOn()
{
    uint32_t ledMask = LED_ONE | LED_TWO | LED_THREE | LED_FOUR;

    qDebug() << Q_FUNC_INFO;

    if (serial_port) {
        RECVCMD_t   cmd = {
            RECVCMD_LEDON,
            ledMask
        };
        serial_port->write(reinterpret_cast<const char*>(&cmd), sizeof(cmd));
        serial_port->flush();
    }
}

void PortMonitor::TurnAllLEDsOff()
{
    uint32_t ledMask = LED_ONE | LED_TWO | LED_THREE | LED_FOUR;


    qDebug() << Q_FUNC_INFO;

    if (serial_port) {
        RECVCMD_t   cmd = {
            RECVCMD_LEDOFF,
            ledMask
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

void PortMonitor::SetCurrentTimeRTC()
{
    QDateTime currentDT = QDateTime::currentDateTime();
    QTime     currentTime = currentDT.time();

    RECVCMD_t cmd;

    uint8_t hBCD = decimal_to_bcd(static_cast<uint8_t>(currentTime.hour()));
    uint8_t mBCD = decimal_to_bcd(static_cast<uint8_t>(currentTime.minute()));
    uint8_t sBCD = decimal_to_bcd(static_cast<uint8_t>(currentTime.second()));

    cmd.cmd  = RECVCMD_SETRTC;
    cmd.data = static_cast<uint32_t>(hBCD << 16) | static_cast<uint32_t>(mBCD << 8) | static_cast<uint32_t>(sBCD);

    serial_port->write(reinterpret_cast<const char*>(&cmd), sizeof(cmd));
    serial_port->flush();
}

qint32 PortMonitor::getId() const
{
    return id;
}

QString PortMonitor::getVersion() const
{
    return version;
}

qint32 PortMonitor::getButton() const
{
    return button;
}

qint32 PortMonitor::getLed() const
{
    return led;
}

QString PortMonitor::getTimestamp() const
{
    return timestamp;
}

QString PortMonitor::getDeviceMsg() const
{
    return deviceMsg;
}
