#ifndef QPATIENT_H
#define QPATIENT_H

#include <QDataStream>
#include <QMap>
#include <QDebug>
#include <QFileInfo>
#include <QtSql>
#include <string>
#include <time.h>

QDateTime TimeT2QDateTime(time_t t);

class QRecord{
public:
    int check_flag = 0; // 1 = valid BrainLab file, 0 if not
    long file_size;
    QString id;
    QString name;
    QDateTime record_start;
    int record_duration_s;
    int sex;
    QString class_code;
    QString protocol;
    QString doctor;
    QString file_name;
    QString file_path;
    QString system;
    int recording_flag;
    int video_flag;
    //int num_pages; // replace with file_duration
    void setID(std::string);
    void setPath(QString);
    void set_values_from_db(QSqlRecord);
};

class QPatient{
public:
    QString id;
    QString name;
    int sex;
    int no; // no of records
    QDateTime last_record; // date of last EEG
    QMap<QString, QRecord> Qrecords_map;
    void set_values(QRecord);
    void add_record(QRecord);
    void set_values_from_db(QSqlRecord);
};

QDataStream & operator<<(QDataStream & out, const QPatient & Qpatient);
QDataStream & operator<<(QDataStream & out, const QRecord & Qrecord);
QDataStream & operator>>(QDataStream & in, QRecord & Qrecord);
QDataStream & operator>>(QDataStream & in, QPatient & Qpatient);

class n_options{

public:
    // general
    int months2load = 24;
    bool boldParent = false;

    // refreshing settings
    int refreshingPeriod = 15; // in minutes
    bool periodicRefreshingEnabled = false;
    int periodicRefreshMode = 0;
    bool refreshWorkingHoursOnly = false;
    bool refreshLoadStatic = true;

    // EEG reader
    QString externalProgram1; // for regular files, scan.exe. in XP "D:/Dropbox/Scripts/Cpp/EEGLE/build-EEGle-Desktop_Qt_5_15_2_MinGW_64_bit-Release/EEGle.exe";
    QString externalProgram2; // for files being recorded - control.exe in XP
    QString defaultReaderFolder;

    // EDF export
    QString exportProgram;
    QString exportPath;
    bool exportAnonymize = false;
    bool exportAllow = true;
    bool exportShortenLabels = false;
    bool exportSystemEvents = false;
    bool exportEnableDebug = false;

    // EEG folders
    QString defaultDataFolder;

    // user editing of db
    bool recordDeleteAllow = false;
};

#endif // QPATIENT_H
