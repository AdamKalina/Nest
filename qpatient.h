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
    int sex = 2; // 0 = female, 1 = male, 2 = unknown
    QString class_code;
    QString protocol;
    QString doctor; // called doctor in the original brainlab files, contains notes from NicoletOne and Harmonie
    QString file_name;
    QString file_path;
    QString recording_system;
    QString guidStudyID; // Nicolet Files only
    int recording_flag;
    int video_flag;
    //int num_pages; // replace with file_duration
    void setID(std::string);
    void setPath(QString);
    void sexFromID(std::string id);
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


// not used anymore
QDataStream & operator<<(QDataStream & out, const QPatient & Qpatient);
QDataStream & operator<<(QDataStream & out, const QRecord & Qrecord);
QDataStream & operator>>(QDataStream & in, QRecord & Qrecord);
QDataStream & operator>>(QDataStream & in, QPatient & Qpatient);

#endif // QPATIENT_H
