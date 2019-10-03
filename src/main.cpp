#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //  Application version
    a.setApplicationVersion(APP_VERSION);

    MainWindow w;
    w.show();

    return a.exec();
}
