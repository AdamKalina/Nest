#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "czech");
    QApplication a(argc, argv);
    QApplication::setWindowIcon(QIcon(":/images/nest_icon.png"));

    QPixmap pixmap(":images/nest.png");

    QPainter p(&pixmap);
    QFont sansFont("Century Gothic", 10);
    p.setFont(sansFont);
    p.setPen(Qt::black);
    p.drawText(200, 340, 300, 30, Qt::AlignLeft | Qt::TextSingleLine, "version 0.2");

    QSplashScreen splash(pixmap, Qt::WindowStaysOnTopHint);

    QTimer t1;
    t1.setSingleShot(true);
    QObject::connect(&t1, SIGNAL(timeout()), &splash, SLOT(close()));
    splash.show();

    t1.start(1500); // show the splash screen for 1500 ms
    QEventLoop evlp;
    QTimer::singleShot(100, &evlp, SLOT(quit()));
    evlp.exec();

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
