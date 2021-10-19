#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
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
#include <QMessageBox>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include "read_signal_file.h"
#include <stdio.h>
#include <string.h>

class Patient{
public:
    string id;
    string name;
    string sex;
    int no; // no of records
    time_t last_record; // date of last EEG
    //vector<Record> records;
    std::map<string, Record> records_map;
    void set_values(Record);
    void add_record(Record);
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    //virtual void paintEvent(QPaintEvent *event);
    std::map<string, Patient> mymap;
    string convert_time_for_sorting(const time_t * timer);
    QString externalProgram;// = "D:/Dropbox/Scripts/Cpp/EEGLE/build-EEGle-Desktop_Qt_5_15_2_MinGW_64_bit-Release/EEGle.exe";
    //QString externalProgram;
    QString stat_dir;// = "D:/Dropbox/Scripts/Cpp/no_data_test"; //static directory
    QString dyn_dir; //dynamic directory
    void readSettings();
    void writeSettings();
    void loadData();
    void buildTreeWidget();
    void showNoFileWarning();

public slots:
    void double_click_record(QTreeWidgetItem* Item);
    void AddFolderDialog();
    void chooseExternalProgram();
    void filter_text_changed(const QString & text);

private:
    QMenuBar *menubar;
    QMenu   *filemenu;
    QBrush *ligh_grey_brush;
    QTreeWidget *treeWidget;

};
#endif // MAINWINDOW_H
