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
#include "read_signal_file.h"
#include "qpatient.h"
#include "leaffilterproxymodel.h"
#include <stdio.h>
#include <string.h>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    //virtual void paintEvent(QPaintEvent *event);
    std::map<string, Patient> mymap;
    QMap<QString, QPatient> patientMap;
    string convert_time_for_sorting(const time_t * timer);
    QString externalProgram;// = "D:/Dropbox/Scripts/Cpp/EEGLE/build-EEGle-Desktop_Qt_5_15_2_MinGW_64_bit-Release/EEGle.exe";
    //QString stat_dir;// = "D:/Dropbox/Scripts/Cpp/no_data_test"; //static directory
    //QString dyn_dir; //dynamic directory
    QString new_dir;
    QString QMapFile;
    QStringList static_dirs;
    QStringList dynamic_dirs;
    int sourceModelLoaded = 0;
    QString defaultDataFolder;
    QString defaultReaderFolder;
    void readSettings();
    void writeSettings();
    void initLoadData();
    void loadData(QString path2load);
    void buildTreeView();
    void updatePatientTreeModel();
    void showNoFileWarning();
    void AddFolderDialog(QString folder_type);
    void buildFilterLine();
    void saveQMap();
    int loadQMap();
    void saveMap();
    long no_files_loaded = 0;


public slots:
    void double_click_tree(QModelIndex index);
    void AddDynamicFolderDialog();
    void AddStaticFolderDialog();
    void chooseExternalProgram();
    void filter_text_changed(const QString & text);
    void refreshDynamic();
    void refreshStatic();
    void notYetReady();
    void show_about_dialog();

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


};
#endif // MAINWINDOW_H
