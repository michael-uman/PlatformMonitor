#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QJsonDocument>

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

private slots:
    void on_LedOnButton_clicked();
    void on_LedOffButton_clicked();


    void on_ledOneOn_clicked();
    void on_ledOneOff_clicked();
    void on_ledTwoOn_clicked();
    void on_ledTwoOff_clicked();
    void on_ledThreeOn_clicked();
    void on_ledThreeOff_clicked();
    void on_ledFourOn_clicked();
    void on_ledFourOff_clicked();

private:
    bool                bOk             = false;
    bool                ledStateInit    = false;
    Ui::MainWindow *    ui              = nullptr;
    PortMonitor *       port_monitor    = nullptr;
};

#endif // MAINWINDOW_H
