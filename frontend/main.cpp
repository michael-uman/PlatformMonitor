#include "mainwindow.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    int result = 10;
    if (w.isOk()) {
        w.show();
        result = a.exec();
    } else {
        qDebug() << "ERROR: Exiting application";
    }
    return result;
}
