#-------------------------------------------------
#
# Project created by QtCreator 2016-06-08T10:42:57
#
#-------------------------------------------------

QT       += core gui sql network xml serialport qml
CONFIG   += c++17

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = tarpuqJIGsw
TEMPLATE = app

SOURCES += src/main.cpp\
        src/messagesform.cpp \
        src/asynccommandlistform.cpp \
        src/dutinfoform.cpp \
        src/dutsummaryform.cpp \
        src/console.cpp \
        src/commandeditdialog.cpp \
        src/duteditdialog.cpp \
        src/gtablewidget.cpp \
        src/gtreewidget.cpp \
        src/interfaceeditdialog.cpp \
        src/jigasynccommand.cpp \
        src/jiginterfaceapp.cpp \
        src/jiginterfacepickit.cpp \
        src/jiginterfaceplugin.cpp \
        src/jiginterfacetty.cpp \
        src/jiginterface.cpp \
        src/jigprofile.cpp \
        src/jigsynccommand.cpp \
        src/logindialog.cpp \
        src/mainwindow.cpp \
        src/pickit.cpp \
        src/loraserverapi.cpp \
        src/synccommandlistform.cpp

HEADERS  += src/mainwindow.h \
        src/messagesform.h \
        src/asynccommandlistform.h \
        src/dutinfoform.h \
        src/dutsummaryform.h \
        src/console.h \
        src/commandeditdialog.h \
        src/duteditdialog.h \
        src/gtablewidget.h \
        src/gtreewidget.h \
        src/jigasynccommand.h \
        src/jiginterfaceapp.h \
        src/jiginterfaceeditdialog.h \
        src/jiginterfacepickit.h \
        src/jiginterfaceplugin.h \
        src/jiginterfacetty.h \
        src/jiginterface.h \
        src/jiglogindialog.h \
        src/jigprofile.h \
        src/jigsynccommand.h \
        src/pickit.h \
        src/loraserverapi.h \
        src/synccommandlistform.h

FORMS    += src/mainwindow.ui \
            src/messagesform.ui \
            src/asynccommandlistform.ui \
            src/dutinfoform.ui \
            src/dutsummaryform.ui \
            src/console.ui \
            src/commandeditdialog.ui \
            src/duteditdialog.ui \
            src/interfaceeditdialog.ui \
            src/logindialog.ui \
            src/synccommandlistform.ui

RESOURCES += \
    src/resources.qrc

INCLUDEPATH += src

win32{
    CONFIG(debug, debug|release) {
        DEST_DIRECTORY = $$PWD/windows/debug
    }
    CONFIG(release, debug|release) {
        DEST_DIRECTORY = $$PWD/windows/release
    }
}

unix:!mac{
    CONFIG(debug, debug|release) {
        DEST_DIRECTORY = $$PWD/linux/debug
    }
    CONFIG(release, debug|release) {
        DEST_DIRECTORY = $$PWD/linux/release
    }
}

MOC_DIR     += $${DEST_DIRECTORY}/generated/mocs
UI_DIR      += $${DEST_DIRECTORY}/generated/uis
RCC_DIR     += $${DEST_DIRECTORY}/generated/rccs
OBJECTS_DIR += $${DEST_DIRECTORY}/generated/objs
DLLDESTDIR = $${DEST_DIRECTORY}/bin
DESTDIR    = $${DEST_DIRECTORY}/bin

# The application version
VERSION = 2020.01.14

# Define the preprocessor macro to get the application version in our application.
DEFINES += APP_VERSION=\\\"$$VERSION\\\"
