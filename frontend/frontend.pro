#-------------------------------------------------
#
# Project created by QtCreator 2019-06-27T14:11:09
#
#-------------------------------------------------

QT       += core gui serialport multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = frontend
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

INCLUDEPATH += \
    ../STM32-TrueStudio-FreeRTOS/Src

SOURCES += \
        bcdstuff.cpp \
        main.cpp \
        mainwindow.cpp \
        portmonitor.cpp

HEADERS += \
        bcdstuff.h \
        mainwindow.h \
        portmonitor.h

FORMS += \
        mainwindow.ui

#win32 {
#    RC_FILE = frontend.rc
#}

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    frontend.rc \
    frontend.ico

RESOURCES += \
    frontend.qrc
