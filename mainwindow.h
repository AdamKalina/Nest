#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QColor>
#include <QDir>
#include <QDebug>
#include <QPainter>
#include <QRect>
#include <QTextDocument>
#include <QTextCodec>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QHeaderView>
#include <QTimer>
#include <QProcess>
#include <QDirIterator>
#include <QSettings>
#include <QLabel>
#include <QMenuBar>
#include <QFileDialog>
#include <QDateTime>
#include <QMessageBox>
#include <QLineEdit>
#include <QDate>
#include <QTime>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QSizePolicy>
#include <QElapsedTimer>
#include <QDataStream>
#include <QFile>
#include <QShortcut>
#include <QSplashScreen>
#include <QStack>
#include <QListWidget>
#include <QPushButton>
#include <QProgressDialog>
#include <QProgressBar>
#include <QQueue>
#include "read_signal_file.h"
#include "qpatient.h"
#include "folderlist.h"
#include "externalprogramlist.h"
#include "leaffilterproxymodel.h"
#include "refreshsettings.h"
#include "customdelegate.h"
#include "dbmanager.h"
#include <stdio.h>
#include <string.h>

class DbManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // std variables
    bool sourceModelLoaded = false;
    bool dbLoaded = false;
    int refreshingPeriod = 15; // in minutes
    bool periodicRefreshingEnabled = false;
    bool workingHoursOnly = false;
    long no_files_loaded = 0;
    int periodicRefreshMode = 0;

    // Qt variables
    QMap<QString, QPatient> patientMap;
    QMap<QString,bool> IdMap;
    QStack<QRecord> QrecordStack;
    QStack<QPatient> QpatientStack;
    QStringList static_dirs;
    QStringList dynamic_dirs;
    QString externalProgram1; // for regular files, scan.exe. in XP "D:/Dropbox/Scripts/Cpp/EEGLE/build-EEGle-Desktop_Qt_5_15_2_MinGW_64_bit-Release/EEGle.exe";
    QString externalProgram2;// for files being recorded - control.exe in XP
    QString new_dir;
    QString QMapFile;
    QString defaultDataFolder;
    QString defaultReaderFolder;
    QString path2db = "records.db";
    QDateTime lastRefreshTime;
    QIcon dvicon;
    DbManager db;

    // functions
    void readSettings();
    void writeSettings();
    void initLoadData();
    void loadDataFromDb();
    void loadDataFromHDD(QString path2load, bool dynamic);
    void updateLastRefreshTime();
    void setUpRefreshQTimer();
    void refreshQTimer();
    void setUpWorkingHoursQTimer();
    void workingHoursQTimer();
    void buildTreeView();
    QAbstractItemModel* createPatientTreeModel();
    void rebuildPatientTreeModel();
    void updatePatientTreeModel();
    void incrementParentNo(QModelIndex parentInd);
    void updateParentTime(QModelIndex parentInd);
    void showNoFileWarning();
    void AddFolderDialog(bool dynamic);
    void buildFilterLine();
    void connectDb();
    void saveQMap();
    int loadQMap();
    //QDateTime TimeT2QDateTime(time_t);


public slots:
    void double_click_tree(QModelIndex index);
    void AddDynamicFolderDialog();
    void AddStaticFolderDialog();
    void chooseExternalProgram1();
    void chooseExternalProgram2();
    void filter_text_changed(const QString & text);
    void refreshDynamic();
    void refreshStatic();
    void notYetReady();
    void show_about_dialog();
    void connect2storage();
    void editFolderList();
    void editProgramList();
    void editRefreshSettings();
    void isItWorkingHours();
    void isItTimeToRefresh();

private:
    QMenuBar *menubar;
    QMenu   *filemenu;
    QMenu *setmenu;
    QMenu *helpmenu;
    QBrush *ligh_grey_brush;
    QTreeView *treeView;
    QStandardItemModel *model;
    LeafFilterProxyModel *proxyModel;
    QAbstractItemModel *sourceModel;
    QLineEdit *filter;
    QWidget *centralWidget;
    QVBoxLayout *layout;
    QShortcut *refreshKey;
    QTimer *timer;
    QTimer *whTimer;

};
#endif // MAINWINDOW_H
