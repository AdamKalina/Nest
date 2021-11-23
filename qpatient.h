#ifndef QPATIENT_H
#define QPATIENT_H

#include <QDataStream>
#include <QMap>
#include <QDebug>
#include <QFileInfo>
#include <QtSql>
#include <string>
#include <time.h>


class QRecord{
public:
    int check_flag = 0; // 1 = valid BrainLab file, 0 if not
    long file_size;
    QString id;
    QString name;
    time_t record_start;
    int sex;
    QString class_code;
    QString protocol;
    QString doctor;
    QString file_name;
    QString file_path;
    int recording_flag;
    int video_flag;
    int num_pages;
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
    time_t last_record; // date of last EEG
    QMap<QString, QRecord> Qrecords_map;
    void set_values(QRecord);
    void add_record(QRecord);
    void set_values_from_db(QSqlRecord);
};

QDataStream & operator<<(QDataStream & out, const QPatient & Qpatient);
QDataStream & operator<<(QDataStream & out, const QRecord & Qrecord);
QDataStream & operator>>(QDataStream & in, QRecord & Qrecord);
QDataStream & operator>>(QDataStream & in, QPatient & Qpatient);

#endif // QPATIENT_H
