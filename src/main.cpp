#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //  Application version
    a.setApplicationVersion(APP_VERSION);
    a.setApplicationName("tarpuqJIGsw");

    MainWindow w;
    w.showMaximized();

    return a.exec();
}
