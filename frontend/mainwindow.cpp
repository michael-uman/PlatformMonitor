#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPortInfo>
#include <QList>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
//#include <QThread>
#include <QSettings>
#include <QDebug>
#include "portmonitor.h"
#include "commands.h"

/**
 * @brief The main window of the frontend application.
 * @param parent
 */
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    QSettings   settings("wunderbar", "frontend");

    ui->setupUi(this);
    setWindowTitle(QString("Frontend V %1").arg("1.0"));

    port_monitor = new PortMonitor();

    if (port_monitor->isOk()) {
        connect(port_monitor, SIGNAL(idChanged(qint32)),
                this, SLOT(onIdChanged(qint32)));
        connect(port_monitor, SIGNAL(buttonChanged(qint32)),
                this, SLOT(onButtonChanged(qint32)));
        connect(port_monitor, SIGNAL(ledChanged(qint32)),
                this, SLOT(onLedChanged(qint32)));
        connect(port_monitor, SIGNAL(versionChanged(QString)),
                this, SLOT(onVersionChanged(QString)));
        connect(port_monitor, SIGNAL(tsChanged(QString)),
                this, SLOT(onTimeChanged(QString)));
        connect(port_monitor, SIGNAL(deviceMsgChanged(QString)),
                this, SLOT(onDeviceMsg(QString)));

        // The switch sound is in the Qt Resource file
        switch_sound = new QSound(":/switch-1.wav");
        if (switch_sound == nullptr) {
            qDebug() << "ERROR: Unable to load sound!";
        }

        bPlaySound = settings.value("soundenabled").toBool();

        ui->actionSound_Enabled->setChecked(bPlaySound);
        bOk = true;
    } else {
        QMessageBox::critical(this, "Device Unavailable", "Unable to locate the device!");
        qDebug() << "ERROR: Unable to open port monitor!";
    }
}

MainWindow::~MainWindow()
{
    QSettings   settings("wunderbar", "frontend");

    settings.setValue("soundenabled", QVariant::fromValue(bPlaySound));

    delete switch_sound;
    delete port_monitor;
    delete ui;
}

bool MainWindow::isOk() const
{
    return bOk;
}

void MainWindow::onIdChanged(qint32 id)
{
    ui->msgIDLabel->setText(QString::number(id));
}

void MainWindow::onButtonChanged(qint32 button)
{
    ui->msgButtonLabel->setText((button == 1)?"Pressed":"Open");
}

void MainWindow::onLedChanged(qint32 led)
{
    QString     sText;

    sText = QString::asprintf("[%c] [%c] [%c] [%c]",
                             IS_LED_SET(led, LED_ONE)?'*':' ',
                             IS_LED_SET(led, LED_TWO)?'*':' ',
                             IS_LED_SET(led, LED_THREE)?'*':' ',
                             IS_LED_SET(led, LED_FOUR)?'*':' ');

    ui->msgLEDLabel->setText(sText);

    // The first time an update is received we reflect
    // the state of the led's in the radio buttons.

//    if (ledStateInit == false) {
        if (IS_LED_SET(led, LED_ONE)) {
            ui->ledOneOn->setChecked(true);
            ui->ledOneOff->setChecked(false);
        } else {
            ui->ledOneOn->setChecked(false);
            ui->ledOneOff->setChecked(true);
        }
        if (IS_LED_SET(led, LED_TWO)) {
            ui->ledTwoOn->setChecked(true);
            ui->ledTwoOff->setChecked(false);
        } else {
            ui->ledTwoOn->setChecked(false);
            ui->ledTwoOff->setChecked(true);
        }
        if (IS_LED_SET(led, LED_THREE)) {
            ui->ledThreeOn->setChecked(true);
            ui->ledThreeOff->setChecked(false);
        } else {
            ui->ledThreeOn->setChecked(false);
            ui->ledThreeOff->setChecked(true);
        }
        if (IS_LED_SET(led, LED_FOUR)) {
            ui->ledFourOn->setChecked(true);
            ui->ledFourOff->setChecked(false);
        } else {
            ui->ledFourOn->setChecked(false);
            ui->ledFourOff->setChecked(true);
        }
        ledStateInit = true;
//    }
}

void MainWindow::onVersionChanged(QString version)
{
    ui->msgFWVersion->setText(version);
}

void MainWindow::onTimeChanged(QString timestamp)
{
    ui->msgTimeLabel->setText(timestamp);
}

void MainWindow::onDeviceMsg(QString message)
{
    ui->deviceLog->append(message);
}

void MainWindow::on_ledOneOn_clicked()
{
    playsound(0);
    port_monitor->TurnLEDOn(0);
}

void MainWindow::on_ledOneOff_clicked()
{
    playsound(0);
    port_monitor->TurnLEDOff(0);
}

void MainWindow::on_ledTwoOn_clicked()
{
    playsound(0);
    port_monitor->TurnLEDOn(1);
}

void MainWindow::on_ledTwoOff_clicked()
{
    playsound(0);
    port_monitor->TurnLEDOff(1);
}

void MainWindow::on_ledThreeOn_clicked()
{
    playsound(0);
    port_monitor->TurnLEDOn(2);
}

void MainWindow::on_ledThreeOff_clicked()
{
    playsound(0);
    port_monitor->TurnLEDOff(2);
}

void MainWindow::on_ledFourOn_clicked()
{
    playsound(0);
    port_monitor->TurnLEDOn(3);
}

void MainWindow::on_ledFourOff_clicked()
{
    playsound(0);
    port_monitor->TurnLEDOff(3);
}

void MainWindow::on_actionSet_RTC_triggered()
{
    qDebug() << "Set RTC";
    port_monitor->SetCurrentTimeRTC();
}

void MainWindow::on_actionAll_On_triggered()
{
    qDebug() << Q_FUNC_INFO;

    playsound(0);
    port_monitor->TurnAllLEDsOn();
}

void MainWindow::on_actionAll_Off_triggered()
{
    qDebug() << Q_FUNC_INFO;

    playsound(0);
    port_monitor->TurnAllLEDsOff();
}

void MainWindow::playsound(int soundId)
{
    Q_UNUSED(soundId)
    if (bPlaySound) {
        switch_sound->play();
    }
}

void MainWindow::on_actionSound_Enabled_toggled(bool arg1)
{
    bPlaySound = arg1;
}
