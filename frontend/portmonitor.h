#ifndef PORTMONITOR_H
#define PORTMONITOR_H

#include <QObject>
#include <QSerialPort>

class PortMonitor : public QObject
{
    Q_OBJECT

    Q_PROPERTY(qint32 id READ getId NOTIFY idChanged)
    Q_PROPERTY(QString version READ getVersion NOTIFY versionChanged)
    Q_PROPERTY(qint32 button READ getButton NOTIFY buttonChanged)
    Q_PROPERTY(qint32 led READ getLed NOTIFY ledChanged)
    Q_PROPERTY(QString timestamp READ getTimestamp NOTIFY tsChanged)
    Q_PROPERTY(QString devicemsg READ getDeviceMsg NOTIFY deviceMsgChanged)

public:
    explicit PortMonitor(QObject *parent = nullptr);
    virtual ~PortMonitor();

    bool isOk() const;

    static void DebugSerialPorts();
    QString GetDevicePort();

    void TurnLEDOn(uint32_t led);
    void TurnLEDOff(uint32_t led);
    void TurnAllLEDsOn();
    void TurnAllLEDsOff();
    void SystemReset();

    void GetVersion();

    void SetCurrentTimeRTC();

    qint32 getId() const;
    QString getVersion() const;
    qint32 getButton() const;
    qint32 getLed() const;
    QString getTimestamp() const;
    QString getDeviceMsg() const;

signals:
    /**
     * @brief This signal broadcasts the message received from the device.
     * @param buffer QJsonDocument object reference of parsed buffer.
     */
    void msgReceived(QJsonDocument & buffer);
    /**
     * @brief This signal is broadcast when the parsed message updates the id #.
     * @param id The current id from the device.
     */
    void idChanged(qint32 id);
    /**
     * @brief This signal is broadcast when the version message is parsed and
     * the version # changed.
     *
     * @param version Version string
     */
    void versionChanged(QString version);
    /**
     * @brief This signal is broadcast when update message indicates that the
     * button state has changed.
     *
     * @param button
     */
    void buttonChanged(qint32 button);
    /**
     * @brief This signal is broadcast when update message indicates that the
     * led state has changed.
     *
     * @param led
     */
    void ledChanged(qint32 led);
    /**
     * @brief This signal is broadcast when the timestamp has changed.
     * @param timestamp
     */
    void tsChanged(QString timestamp);

    void deviceMsgChanged(QString message);

public slots:
    void onDataReady();


private:
    QString     port_name;
    QSerialPort * serial_port = nullptr;
    bool        ok          = false;

    void parseJSON(QByteArray & buffer);

    qint32      id          = -1;
    qint32      button      = -1;
    qint32      led         = -1;
    QString     version     = "Unknown";
    QString     timestamp   = "Unknown";
    QString     deviceMsg;
};

#endif // PORTMONITOR_H
