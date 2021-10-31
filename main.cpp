#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "czech");
    QApplication a(argc, argv);
    QApplication::setWindowIcon(QIcon(":/images/nest_icon.png"));
    MainWindow w;

    w.setWindowTitle("EEGLE Nest");
    //w.setWindowIcon(QIcon(":/images/nest_icon.png")); // this sets the icon just for the MainWindow
    //w.setMinimumHeight(820);
    //w.setMinimumWidth(875); // 1200

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
