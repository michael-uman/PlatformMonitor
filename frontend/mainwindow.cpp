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
        QJsonObject obj = doc.object();

        // Does it contain the keys we need?
        if (obj.contains("id") && obj.contains("button")) {
            QString sText;
            int     id     = obj["id"].toInt();
            int     button = obj["button"].toInt();

            sText = QString::asprintf("%d", id);
            ui->msgIDLabel->setText(sText);
            sText = QString::asprintf("%s", (button == 1)?"Pressed":"Open");
            ui->msgButtonLabel->setText(sText);
//            qDebug() << "ID : " << id << " BUTTON : " << button;
        } else {
            qDebug() << "Looks like invalid JSON to me!";
        }

    } else {
        qDebug() << "No JSON Object found!";
    }
}

