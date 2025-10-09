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
    QString brainlab_class_code;
    //QString protocol; // never used
    QString brainlab_doctor; // called doctor in the original brainlab files
    QString nicolet_record_id_file; // original record_id not read from the bile
    QString nicolet_record_id_db; // record_id note read from the nicolet db
    QString file_name;
    QString file_path;
    QString file_id;
    QString recording_system;
    QString comment;
    int recording_flag;
    int video_flag;
    int report_flag;
    //int num_pages; // replace with file_duration
    void setID(std::string);
    void setPath(QString);
    void sexFromID(std::string id);
    void set_values_from_db(QSqlRecord);
    void set_comment();
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

class QReport{
public:
    QString file_id;
    QString file_path;
    QString datum;
    QString rodne_cislo;
    QString jmeno;
    QString odesilajici_lekar_original;
    QString lateralita;
    QString duvod_vysetreni;
    QString uroven_vedomi;
    QString laborant;
    QString popis;
    QString fotostimulace;
    QString tf;
    QString zaver_klasifikace;
    QString klinicka_interpretace;
    QString statisticky_kod_text;

    void set_values_from_db(QSqlRecord);

};


// not used anymore
QDataStream & operator<<(QDataStream & out, const QPatient & Qpatient);
QDataStream & operator<<(QDataStream & out, const QRecord & Qrecord);
QDataStream & operator>>(QDataStream & in, QRecord & Qrecord);
QDataStream & operator>>(QDataStream & in, QPatient & Qpatient);

#endif // QPATIENT_H
