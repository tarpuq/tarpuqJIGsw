#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //  Application version
    a.setOrganizationName("Tarpuq Cia. Ltda.");
    a.setOrganizationDomain("tarpuq-ems.com");
    a.setApplicationName("tarpuqJIGsw");
    a.setApplicationVersion(APP_VERSION);

    MainWindow w;
    w.showMaximized();

    return a.exec();
}
