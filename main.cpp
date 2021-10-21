#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "czech");
    QApplication a(argc, argv);
    MainWindow w;

    w.setWindowTitle("EEGle Nest");
    w.setMinimumHeight(820);
    w.setMinimumWidth(875); // 1200

    //qDebug() << w.sizeHint();

    //w.adjustSize();

    //Set Stylesheet
        QFile qss(":style.qss");

        if (!qss.exists())   {
            printf("Unable to set stylesheet, file not found\n");
        }
        else   {
            qss.open(QFile::ReadOnly | QFile::Text);
            QTextStream ts(&qss);
            qApp->setStyleSheet(ts.readAll());
        }

    w.show();
    return a.exec();
}
