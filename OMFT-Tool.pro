QT       += core gui
QT       += serialport
QT       += network
QT       += xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    FileReadWrite/filerw.cpp \
    FileReadWrite/xmlcommandparser.cpp \
    HttpClient/deviceinfo.cpp \
    HttpClient/httpclient.cpp \
    HttpClient/infoparse.cpp \
    Logs/logs.cpp \
    Modbus/ModbusReMapping.cpp \
    Modbus/ModbusWriteBuild.cpp \
    Modbus/gt_modbus.cpp \
    SerialSource/base_serial_port.cpp \
    SerialSource/serialworker.cpp \
    Table/tablecontrol.cpp \
    Tasks/test_loop.cpp \
    Tasks/testworker.cpp \
    hexprintf.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    FileReadWrite/filerw.h \
    FileReadWrite/xmlcommandparser.h \
    HttpClient/deviceinfo.h \
    HttpClient/httpclient.h \
    HttpClient/infoparse.h \
    Logs/logs.h \
    Modbus/ModbusReMapping.h \
    Modbus/ModbusWriteBuild.h \
    Modbus/gt_modbus.h \
    SerialSource/base_serial_port.h \
    SerialSource/serialworker.h \
    Table/tablecontrol.h \
    Tasks/test_loop.h \
    Tasks/testworker.h \
    hexprintf.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

TRANSLATIONS += \
    OMFT-Tool_zh_CN.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
