#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QSplashScreen>
#include <QTextCodec>
#include <QDebug>
#include <QColor>
#include <QScrollArea>
#include <QVBoxLayout>

#include <QTimer>
#include <QElapsedTimer>
#include <QProcess>
#include <QDirIterator>
#include <QFile>
#include <QSettings>
#include <QLabel>
#include <QMenuBar>
#include <QFileDialog>
#include <QProgressDialog>
#include <QMessageBox>
#include <QFileSystemWatcher>
#include <QLockFile>
#include <QDateTime>

#include <QLineEdit>
#include <QRegExp>
#include <QHeaderView>
#include <QTreeView>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QDataStream>
#include <QShortcut>
#include <QStack>
#include <QQueue>
#include <QListWidget>
#include <QPushButton>

#include "read_signal_file.h"
#include "qpatient.h"
#include "folderlist.h"
#include "externalprogramlist.h"
#include "leaffilterproxymodel.h"
#include "refreshsettings.h"
#include "options_dialog.h"
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
    long no_files_loaded = 0;
    //    int refreshingPeriod = 15; // in minutes
    //    bool periodicRefreshingEnabled = false;
    //    bool workingHoursOnly = false;
    //    bool loadStaticOnRefreshEnabled = true;
    //    bool boldParent = false;

    //    int periodicRefreshMode = 0;
    //    int months2load = 24;

    // Qt variables
    QMap<QString, QPatient> patientMap;
    QMap<QString,bool> IdMap;
    QQueue<QRecord> QrecordQueue;
    QQueue<QPatient> QpatientQueue;
    QQueue<QFileInfo> fiQueue; // queue for filesInfo
    QStringList static_dirs;
    QStringList dynamic_dirs;
    QStringList next_files;
    QString new_dir;
    QString QMapFile;
    QString path2db = "records.db";
    QString brainLabDrive = "S:\\";
    QStringList usedDrives;
    QStringList batchFiles;
    QDateTime lastRefreshTime;
    QIcon dvicon;
    DbManager db;
    n_options nestOptions;

    // functions
    void readSettings();
    void writeSettings();
    void loadDataFromDb();
    void checkFolders(const QStringList dirs, bool dynamic);
    void checkDataOnHDD(QString path2load, bool dynamic);
    void readDataOnHDD(QString path2load, bool dynamic);
    void checkQPatient(QPatient qpatient);
    void updateLastRefreshTime();
    void setUpRefreshQTimer();
    void refreshQTimer();
    void setUpWorkingHoursQTimer();
    void workingHoursQTimer();
    void buildTreeView();
    QAbstractItemModel* createPatientTreeModel();
    QRecord prepareQRecord(QFileInfo fileInfo, bool dynamic);
    QRecord getQRecord(QFileInfo fileInfo);
    void rebuildPatientTreeModel();
    void updatePatientTreeModel();
    void incrementParentNo(QModelIndex parentInd);
    void updateParentTime(QModelIndex parentInd);
    void buildNoFileWarning();
    void AddFolderDialog(bool dynamic);
    void buildFilterLine();
    void connectDb();
    //void saveQMap();
    //int loadQMap();
    //QDateTime TimeT2QDateTime(time_t);
    void initSystemWatcher();
    void double_click_patient(QModelIndex index);
    void double_click_record(QModelIndex index);
    void openBrainLabControl(QString path);
    void runBatchFile(QString batchFile);
    QString getNextFileInFolder(QString path);
    QString getNextFileName(QString lastFile);
    void writeWatcherLog(QString log);
    void closeEvent(QCloseEvent *event);
    void fullTextSearch(QString query);


public slots:
    void double_click_tree(QModelIndex index);
    void AddDynamicFolderDialog();
    void AddStaticFolderDialog();
    void chooseExternalProgram1();
    void chooseExternalProgram2();
    void chooseExportProgram();
    void filter_text_changed(const QString & text);
    void filter_return_pressed();
    void refreshDynamic();
    void refreshStatic();
    void notYetReady();
    void show_about_dialog();
    void connect2storage();
    void editFolderList();
    //void editProgramList();
    //void editRefreshSettings();
    void editTabOptions();
    void isItWorkingHours();
    void isItTimeToRefresh();
    void collapseAll();
    void expandAll();
    void showPath();
    void recordedFileChanged(const QString & path);
    void ShowContextMenu(const QPoint &);
    void exportToEDF();

private:
    QMenuBar *menubar;
    QMenu   *filemenu, *setmenu, *helpmenu, *viewmenu;
    QBrush *ligh_grey_brush;
    QTreeView *treeView;
    QStandardItemModel *model;
    LeafFilterProxyModel *proxyModel;
    QAbstractItemModel *sourceModel;
    QLineEdit *filter;
    QWidget *centralWidget;
    QVBoxLayout *layout;
    QShortcut *refreshKey, *helpKey;
    QTimer *timer;
    QTimer *whTimer;
    QAction *showPathAction, *refreshDynamicAction;
    QFileSystemWatcher * watcher, * recordingFileWatcher;

};
#endif // MAINWINDOW_H
