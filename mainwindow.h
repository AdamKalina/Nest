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
    QString externalProgram = "D:/Dropbox/Scripts/Cpp/EEGLE/build-EEGle-Desktop_Qt_5_15_2_MinGW_64_bit-Release/EEGle.exe";

public slots:
    void click_test(QTreeWidgetItem* Item);

};
#endif // MAINWINDOW_H
