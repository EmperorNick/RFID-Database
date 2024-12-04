QT       += core gui
QT       += sql #should add sql
QT       += mqtt #should add mqtt

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
LIBS += -lwiringPi
LIBS += -lbcm2835


# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    databasedialog.cpp \
    main.cpp \
    mainwindow.cpp \
    mqttmanager.cpp \
    writedialog.cpp

HEADERS += \
    databasedialog.h \
    mainwindow.h \
    mqttmanager.h \
    writedialog.h

FORMS += \
    databasedialog.ui \
    mainwindow.ui \
    writedialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    MFRC522.py \
    RFIDScan.py
