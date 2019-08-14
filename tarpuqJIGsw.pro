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
        src/jiginterfacepickit.cpp \
        src/jiginterfaceplugin.cpp \
        src/jiginterfacetty.cpp \
        src/jiginterfaceedit.cpp \
        src/jiginterface.cpp \
        src/jigsaw.cpp \
        src/mainwindow.cpp \
        src/pickit.cpp \
        src/loraserverapi.cpp \
        src/jigcommand.cpp \
        src/jigcommandedit.cpp

HEADERS  += src/mainwindow.h \
        src/jiginterfacepickit.h \
        src/jiginterfaceplugin.h \
        src/jiginterfacetty.h \
        src/jiginterfaceedit.h \
        src/jiginterface.h \
        src/jigsaw.h \
        src/pickit.h \
        src/loraserverapi.h \
        src/jigcommand.h \
        src/jigcommandedit.h

FORMS    += src/mainwindow.ui \
            src/jiginterfaceedit.ui \
            src/jigcommandedit.ui

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

