#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "czech");
    QApplication a(argc, argv);
    MainWindow w;

    w.setWindowTitle("EEGLE Nest");
    w.setMinimumHeight(820);
   w.setMinimumWidth(750); // 1200

    //qDebug() << w.sizeHint();

    //w.adjustSize();

    w.show();
    return a.exec();
}
