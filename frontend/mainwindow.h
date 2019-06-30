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

public slots:
    void onMessageRecv(QJsonDocument & doc);

private slots:
    void on_LedOnButton_clicked();

    void on_LedOffButton_clicked();

    void on_versionButton_clicked();

private:
    Ui::MainWindow *    ui              = nullptr;
    PortMonitor *       port_monitor    = nullptr;
};

#endif // MAINWINDOW_H
