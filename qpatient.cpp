#include "qpatient.h"

// ======== CLASS for QPATIENT and STREAMER methods ========

void Patient::set_values (Record record) {
    id = record.id;
    name = record.name;
    sex = record.sex;
    no = 1;
    last_record = record.record_start;
    //records.push_back(record);
    records_map.insert(std::pair<string,Record>(record.file_name,record));
};

void Patient::add_record(Record record){
    // TO DO - check for duplicates?
    //records.push_back(record);
    records_map.insert(std::pair<string,Record>(record.file_name,record));

    // compares start of recordings and uses later
    if (difftime(last_record, record.record_start) < 0){
        last_record = record.record_start;
    };

    no = records_map.size(); // beter than no++ since it would increment even in duplicate files
};

void QPatient::set_values(Record record){
    id = QString::fromLocal8Bit(record.id.c_str());
    name = QString::fromLocal8Bit(record.name.c_str());
    sex = record.sex;
    no = 1;
    last_record = record.record_start;
}

void QPatient::add_record(Record record){

    QRecord Qrecord;

    Qrecord.check_flag = record.check_flag;
    Qrecord.file_size = record.file_size;
    Qrecord.id = QString::fromLocal8Bit(record.id.c_str());
    Qrecord.name = QString::fromLocal8Bit(record.name.c_str());
    Qrecord.record_start = record.record_start;
    Qrecord.sex = record.sex;
    Qrecord.class_code = QString::fromLocal8Bit(record.class_code.c_str());
    Qrecord.protocol = QString::fromLocal8Bit(record.protocol.c_str());
    Qrecord.file_name = QString::fromLocal8Bit(record.file_name.c_str());
    Qrecord.file_path = QString::fromLocal8Bit(record.file_path.c_str());
    Qrecord.recording_flag = record.recording_flag;
    Qrecord.video_flag = record.video_flag;

    Qrecords_map.insert(Qrecord.file_name,Qrecord);

    // compares start of recordings and uses later
    if (difftime(last_record, Qrecord.record_start) < 0){
        last_record = Qrecord.record_start;
    };

    no = Qrecords_map.size();
};

QDataStream & operator<<(QDataStream & out, const QPatient & Qpatient)
{
    out << Qpatient.id << (qint32)Qpatient.last_record << Qpatient.name << Qpatient.sex << Qpatient.Qrecords_map;
    return out;
}

QDataStream & operator<<(QDataStream & out, const QRecord & Qrecord)
{
    out << Qrecord.check_flag << Qrecord.class_code << Qrecord.file_name << Qrecord.file_path << Qrecord.file_size << Qrecord.id << Qrecord.name << Qrecord.protocol << Qrecord.record_start << Qrecord.recording_flag << Qrecord.sex << Qrecord.video_flag;
    return out;
}


QDataStream & operator>>(QDataStream & in, QPatient & Qpatient)
{
    in >> Qpatient.id >> (qint32&)Qpatient.last_record >> Qpatient.name >> Qpatient.sex >> Qpatient.Qrecords_map;
    qDebug() << Qpatient.name;
    return in;
}

QDataStream & operator>>(QDataStream & in, QRecord & Qrecord)
{
    in >> Qrecord.check_flag >> Qrecord.class_code >> Qrecord.file_name >> Qrecord.file_path >> Qrecord.file_size >> Qrecord.id >> Qrecord.name >> Qrecord.protocol >> Qrecord.record_start >> Qrecord.recording_flag >> Qrecord.sex >> Qrecord.video_flag;
    qDebug() << Qrecord.file_path;
    return in;
}
