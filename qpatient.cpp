#include "qpatient.h"

// ======== CLASS for QPATIENT and STREAMER methods ========


void QRecord::setID(std::string old_id){
    // removes "/" in ID (rodné èíslo)
    id = QString::fromLocal8Bit(old_id.c_str());
    id.remove(QChar('/'), Qt::CaseInsensitive);
    //capitalize "x" ind ID
    id.replace("x", "X", Qt::CaseSensitive);
}

void QRecord::setPath(QString old_path){
    file_path = old_path;
    // extract file name
    QFileInfo fi(file_path);
    file_name = fi.baseName();
}

// TO DO - make Time_t --> QDateTime conversion part of QRecord constructor ?

void QPatient::set_values(QRecord qrecord){
    id = qrecord.id;
    name = qrecord.name;
    sex = qrecord.sex;
    no = 1;
    last_record = qrecord.record_start;
    this->add_record(qrecord);
}

void QPatient::add_record(QRecord Qrecord){

    Qrecords_map.insert(Qrecord.file_name,Qrecord);

    // compares start of recordings and uses later
    if (difftime(last_record, Qrecord.record_start) < 0){
        last_record = Qrecord.record_start;
    };

    no = Qrecords_map.size();
};

QDataStream & operator<<(QDataStream & out, const QPatient & Qpatient)
{
    out << Qpatient.id << (qint32)Qpatient.last_record << Qpatient.name << Qpatient.sex << (qint16)Qpatient.no << Qpatient.Qrecords_map;
    //qDebug() << "no out:" << Qpatient.no;
    return out;
}

QDataStream & operator<<(QDataStream & out, const QRecord & Qrecord)
{
    out << Qrecord.check_flag << Qrecord.class_code << Qrecord.file_name << Qrecord.file_path;
    out << (quint32)Qrecord.file_size << Qrecord.id << Qrecord.name << Qrecord.protocol << Qrecord.record_start;
    out << Qrecord.recording_flag << Qrecord.sex << Qrecord.video_flag << (qint16)Qrecord.num_pages;
    //qDebug() << "pages out: " << Qrecord.num_pages;
    return out;
}


QDataStream & operator>>(QDataStream & in, QPatient & Qpatient)
{
    in >> Qpatient.id >> (qint32&)Qpatient.last_record >> Qpatient.name >> Qpatient.sex;
    qint16 no;
    in >> no;
    Qpatient.no = no; // do I have to do it inline?

    //in >> (qint16&)Qpatient.no;
    in >> Qpatient.Qrecords_map;
    //qDebug() << "no in:" << (qint16&)Qpatient.no;
    return in;
}

QDataStream & operator>>(QDataStream & in, QRecord & Qrecord)
{
    in >> Qrecord.check_flag >> Qrecord.class_code >> Qrecord.file_name >> Qrecord.file_path;
    in >> (qint32&)Qrecord.file_size >> Qrecord.id >> Qrecord.name >> Qrecord.protocol >> Qrecord.record_start;
    in >> Qrecord.recording_flag >> Qrecord.sex >> Qrecord.video_flag;

    qint16 pages;
    in >> pages;
    Qrecord.num_pages = pages;
    //in >> (qint16&)Qrecord.num_pages;
    //qDebug() << "pages in: " << (qint16&)Qrecord.num_pages;
    return in;
}
