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
    p.drawText(20, 60, 300, 30, Qt::AlignLeft | Qt::TextSingleLine, "version 0.35");

    QSplashScreen splash(pixmap, Qt::WindowStaysOnTopHint);
    splash.setFont(sansFont);

    QTimer t1;
    t1.setSingleShot(true);

    QObject::connect(&t1, SIGNAL(timeout()), &splash, SLOT(close()));
    splash.show();
    splash.showMessage("Program is starting", Qt::AlignHCenter | Qt::AlignBottom);

    t1.start(1500); // show the splash screen for 1500 ms
    QEventLoop evlp;
    QTimer::singleShot(100, &evlp, SLOT(quit()));
    evlp.exec();

    MainWindow w;
    w.setWindowTitle("EEGLE Nest");

    // ======== APPLICATION CYCLE ========

    splash.showMessage("Reading settings", Qt::AlignHCenter | Qt::AlignBottom);
    w.readSettings();

    splash.showMessage("Connecting to database", Qt::AlignHCenter | Qt::AlignBottom);
    w.connectDb();

    splash.showMessage("Loading data", Qt::AlignHCenter | Qt::AlignBottom);
    w.initLoadData(); //load data and update patientMap

    splash.showMessage("Building tree model", Qt::AlignHCenter | Qt::AlignBottom);
    w.buildTreeView();
    w.setUpRefreshQTimer();
    w.setUpWorkingHoursQTimer();
    w.updateLastCheckTime();

    if (w.patientMap.size() == 0){
        w.showNoFileWarning(); // show warning that there are no files to load
    }

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


    // show mainwindow
    w.show();
    return a.exec();
}
