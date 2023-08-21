#include "mainwindow.h"
#include <QApplication>

#undef main
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.resize(640,480);
    w.show();

    return a.exec();
}
