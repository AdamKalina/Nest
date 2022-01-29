#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "czech");
    QApplication a(argc, argv);
    QApplication::setWindowIcon(QIcon(":/images/nest_icon.png"));

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

    // ======== SPLASH SCREEN ========

    QPixmap pixmap(":images/nest.png");

    QPainter p(&pixmap);
    QFont sansFont("Century Gothic", 10);
    p.setFont(sansFont);
    p.setPen(Qt::black);
    p.drawText(20, 60, 300, 30, Qt::AlignLeft | Qt::TextSingleLine, "version 0.5.3");
    p.drawText(20, 300, 300, 30, Qt::AlignLeft | Qt::TextSingleLine, QString("build %1").arg(__DATE__));

    QSplashScreen splash(pixmap, Qt::WindowStaysOnTopHint);
    splash.setFont(sansFont);

    QTimer t1;
    t1.setSingleShot(true);

    QObject::connect(&t1, SIGNAL(timeout()), &splash, SLOT(close()));
    splash.show();
    splash.showMessage("Program is starting", Qt::AlignHCenter | Qt::AlignBottom);

    t1.start(1500); // show the splash screen for 1500 ms - even after the app is loaded (?)
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
    w.initSystemWatcher(); // initiate watchers before loading data - duh
    w.loadDataFromDb();
    w.initLoadData(); //load data if there is no db, init next_files

    w.setUpRefreshQTimer();
    w.setUpWorkingHoursQTimer();
    splash.showMessage("Looking for new data", Qt::AlignHCenter | Qt::AlignBottom);
    w.refreshDynamic(); // did not really work when part of initLoadData

    splash.clearMessage();

    // =================================
    // show mainwindow
    w.checkQMap(); // if IdMap is empty, it will show warning, otherwise it will show treeview
    w.show();
    return a.exec();
}
