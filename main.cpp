#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "czech");
    QApplication a(argc, argv);


    // use this for single instance application - from here https://evileg.com/en/post/147/
    QLockFile lockFile(QDir::temp().absoluteFilePath("brainlab42.lock"));

    /* Trying to close the Lock File, if the attempt is unsuccessful for 100 milliseconds,
         * then there is a Lock File already created by another process.
         / Therefore, we throw a warning and close the program
         * */
    if(!lockFile.tryLock(100)){
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("The application is already running.\n"
                           "Switch to the opened Nest instance.");
        msgBox.exec();
        return 1;
    }

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
    p.drawText(20, 60, 300, 30, Qt::AlignLeft | Qt::TextSingleLine, "version 0.81");
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

    splash.showMessage("Setting up timers and watchers", Qt::AlignHCenter | Qt::AlignBottom);
    w.initSystemWatcher(); // initiate watchers before loading data - duh
    w.setUpRefreshQTimer();
    w.setUpWorkingHoursQTimer();
    splash.showMessage("Looking for new data", Qt::AlignHCenter | Qt::AlignBottom);
    w.refreshDynamic();
    w.program_is_starting = false;

    splash.showMessage("Loading data from database", Qt::AlignHCenter | Qt::AlignBottom);
    w.loadDataFromDb();
    w.updatePatientTreeModel();

    splash.clearMessage();

    // =================================
    // show mainwindow
    w.show();
    w.checkStorage();
    return a.exec();
}
