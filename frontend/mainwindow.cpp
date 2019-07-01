#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPortInfo>
#include <QList>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QDebug>
#include "portmonitor.h"
#include "commands.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

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
        bOk = true;
    } else {
        QMessageBox::critical(this, "Device Unavailable", "Unable to locate the device!");
        qDebug() << "ERROR: Unable to open port monitor!";
    }
}

MainWindow::~MainWindow()
{
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

    if (ledStateInit == false) {
        if (IS_LED_SET(led, LED_ONE)) {
            ui->ledOneOn->setChecked(true);
        } else {
            ui->ledOneOn->setChecked(false);
        }
        if (IS_LED_SET(led, LED_TWO)) {
            ui->ledTwoOn->setChecked(true);
        } else {
            ui->ledTwoOn->setChecked(false);
        }
        if (IS_LED_SET(led, LED_THREE)) {
            ui->ledThreeOn->setChecked(true);
        } else {
            ui->ledThreeOn->setChecked(false);
        }
        if (IS_LED_SET(led, LED_FOUR)) {
            ui->ledFourOn->setChecked(true);
        } else {
            ui->ledFourOn->setChecked(false);
        }
        ledStateInit = true;
    }
    // update button enable according to LED status
//    ui->LedOnButton->setEnabled(led == 0);
//    ui->LedOffButton->setEnabled(led == 1);
}

void MainWindow::onVersionChanged(QString version)
{
    ui->msgFWVersion->setText(version);
}


void MainWindow::on_LedOnButton_clicked()
{
    port_monitor->TurnLEDOn(0);
}

void MainWindow::on_LedOffButton_clicked()
{
    port_monitor->TurnLEDOff(0);
}



void MainWindow::on_ledOneOn_clicked()
{
    port_monitor->TurnLEDOn(0);
}

void MainWindow::on_ledOneOff_clicked()
{
    port_monitor->TurnLEDOff(0);

}

void MainWindow::on_ledTwoOn_clicked()
{
    port_monitor->TurnLEDOn(1);

}

void MainWindow::on_ledTwoOff_clicked()
{
    port_monitor->TurnLEDOff(1);
}

void MainWindow::on_ledThreeOn_clicked()
{
    port_monitor->TurnLEDOn(2);
}

void MainWindow::on_ledThreeOff_clicked()
{
    port_monitor->TurnLEDOff(2);
}

void MainWindow::on_ledFourOn_clicked()
{
    port_monitor->TurnLEDOn(3);
}

void MainWindow::on_ledFourOff_clicked()
{
    port_monitor->TurnLEDOff(3);
}
