#ifndef PORTMONITOR_H
#define PORTMONITOR_H

#include <QObject>
#include <QSerialPort>

class PortMonitor : public QObject
{
    Q_OBJECT
public:
    explicit PortMonitor(QObject *parent = nullptr);
    static void DebugSerialPorts();

    void TurnLEDOn(uint32_t led);
    void TurnLEDOff(uint32_t led);
    void GetVersion();

signals:
    void msgReceived(QJsonDocument & buffer);

public slots:
    void onDataReady();

private:
    QSerialPort * serial_port = nullptr;
    void parseJSON(QByteArray & buffer);
};

#endif // PORTMONITOR_H
