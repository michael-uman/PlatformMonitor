#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPortInfo>
#include <QList>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include "portmonitor.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    port_monitor = new PortMonitor();
    connect(port_monitor, SIGNAL(msgReceived(QJsonDocument &)), this, SLOT(onMessageRecv(QJsonDocument &)));
}

MainWindow::~MainWindow()
{
    delete port_monitor;
    delete ui;
}

void MainWindow::onMessageRecv(QJsonDocument &doc)
{
    // Verify that we were passed a valid JSON object...
    if (doc.isObject()) {
        QString sText;
        QJsonObject obj = doc.object();

        // Does it contain the keys we need?
        if (obj.contains("version")) {
            sText = obj["version"].toString();
            qDebug() << "Version : " << sText;
            ui->msgFWVersion->setText(sText);
        } else if (obj.contains("message")) {
            sText = obj["message"].toString();
            qDebug() << "DEVICE MSG : " << sText;
        } else if (obj.contains("id") &&
                   obj.contains("button") &&
                   obj.contains("led")) {
            int     id     = obj["id"].toInt();
            int     button = obj["button"].toInt();
            int     led    = obj["led"].toInt();

            sText = QString::asprintf("%d", id);
            ui->msgIDLabel->setText(sText);
            sText = QString::asprintf("%s", (button == 1)?"Pressed":"Open");
            ui->msgButtonLabel->setText(sText);
            sText = QString::asprintf("%s", (led == 1)?"On":"Off");
            ui->msgLEDLabel->setText(sText);

            // update button enable according to LED status
            ui->LedOnButton->setEnabled(led == 0);
            ui->LedOffButton->setEnabled(led == 1);
        } else {
            qDebug() << "Looks like invalid JSON to me!";
        }

    } else {
        qDebug() << "No JSON Object found!";
    }
}


void MainWindow::on_LedOnButton_clicked()
{
    port_monitor->TurnLEDOn(1);
}

void MainWindow::on_LedOffButton_clicked()
{
    port_monitor->TurnLEDOff(1);
}

void MainWindow::on_versionButton_clicked()
{
    port_monitor->GetVersion();
}
