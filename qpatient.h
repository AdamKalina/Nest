#ifndef QPATIENT_H
#define QPATIENT_H

#include <QDataStream>
#include <QMap>
#include <QDebug>
#include <string.h>
#include <stdio.h>
#include <read_signal_file.h>

class Patient{
public:
    string id;
    string name;
    string sex;
    int no; // no of records
    time_t last_record; // date of last EEG
    std::map<string, Record> records_map;
    void set_values(Record);
    void add_record(Record);
};

struct QRecord{
    int check_flag = 0; // 1 = valid BrainLab file, 0 if not
    long file_size;
    QString id;
    QString name;
    time_t record_start;
    short sex;
    QString class_code;
    QString protocol;
    QString file_name;
    QString file_path;
    int recording_flag;
    int video_flag;
    int num_pages;
};

class QPatient{
public:
    QString id;
    QString name;
    QString sex;
    int no; // no of records
    time_t last_record; // date of last EEG
    QMap<QString, QRecord> Qrecords_map;
    void set_values(Record);
    void add_record(Record);
};

QDataStream & operator<<(QDataStream & out, const QPatient & Qpatient);
QDataStream & operator<<(QDataStream & out, const QRecord & Qrecord);
QDataStream & operator>>(QDataStream & in, QRecord & Qrecord);
QDataStream & operator>>(QDataStream & in, QPatient & Qpatient);

#endif // QPATIENT_H
