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
        src/console.cpp \
        src/asynccommandlist.cpp \
        src/commandeditdialog.cpp \
        src/gtablewidget.cpp \
        src/interfaceeditdialog.cpp \
        src/jigasynccommand.cpp \
        src/jiginterfacepickit.cpp \
        src/jiginterfaceplugin.cpp \
        src/jiginterfacetty.cpp \
        src/jiginterface.cpp \
        src/jigsaw.cpp \
        src/jigsynccommand.cpp \
        src/logindialog.cpp \
        src/mainwindow.cpp \
        src/pickit.cpp \
        src/loraserverapi.cpp \
        src/synccommandlist.cpp

HEADERS  += src/mainwindow.h \
        src/console.h \
        src/asynccommandlist.h \
        src/commandeditdialog.h \
        src/gtablewidget.h \
        src/jigasynccommand.h \
        src/jiginterfaceeditdialog.h \
        src/jiginterfacepickit.h \
        src/jiginterfaceplugin.h \
        src/jiginterfacetty.h \
        src/jiginterface.h \
        src/jiglogindialog.h \
        src/jigsaw.h \
        src/jigsynccommand.h \
        src/pickit.h \
        src/loraserverapi.h \
        src/synccommandlist.h

FORMS    += src/mainwindow.ui \
            src/console.ui \
            src/asynccommandlist.ui \
            src/commandeditdialog.ui \
            src/interfaceeditdialog.ui \
            src/logindialog.ui \
            src/synccommandlist.ui

RESOURCES += \
    src/resources.qrc

win32{
    CONFIG(debug, debug|release) {
        DEST_DIRECTORY = $$PWD/win32/debug
    }
    CONFIG(release, debug|release) {
        DEST_DIRECTORY = $$PWD/win32/release
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
DLLDESTDIR = $${DEST_DIRECTORY}
DESTDIR    = $${DEST_DIRECTORY}

# The application version
VERSION = 20191108

# Define the preprocessor macro to get the application version in our application.
DEFINES += APP_VERSION=\\\"$$VERSION\\\"
