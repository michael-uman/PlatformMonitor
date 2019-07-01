#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QJsonDocument>
#include <QSound>

class PortMonitor;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool isOk() const;

public slots:
//    void onMessageRecv(QJsonDocument & doc);
    void onIdChanged(qint32 id);
    void onButtonChanged(qint32 button);
    void onLedChanged(qint32 led);
    void onVersionChanged(QString version);
    void onTimeChanged(QString timestamp);
    void onDeviceMsg(QString message);

private slots:
//    void on_LedOnButton_clicked();
//    void on_LedOffButton_clicked();


    void on_ledOneOn_clicked();
    void on_ledOneOff_clicked();
    void on_ledTwoOn_clicked();
    void on_ledTwoOff_clicked();
    void on_ledThreeOn_clicked();
    void on_ledThreeOff_clicked();
    void on_ledFourOn_clicked();
    void on_ledFourOff_clicked();

    void on_actionSet_RTC_triggered();

    void on_actionAll_On_triggered();

private:
    bool                bOk             = false;
    bool                ledStateInit    = false;
    Ui::MainWindow *    ui              = nullptr;
    PortMonitor *       port_monitor    = nullptr;
    QSound *            switch_sound    = nullptr;
};

#endif // MAINWINDOW_H
