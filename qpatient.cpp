#include "qpatient.h"

// ======== CLASS for QPATIENT and STREAMER methods ========


void QRecord::setID(std::string old_id){
    // removes "/" in ID (rodné èíslo) and switch "x" to upper case
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

void QRecord::set_values_from_db(QSqlRecord rec){
    file_name = rec.value("file_name").toString();
    id = rec.value("id").toString();
    name = rec.value("name").toString();
    record_start = TimeT2QDateTime(rec.value("record_start").toInt());
    record_duration_s = rec.value("record_duration_s").toInt();
    sex = rec.value("sex").toInt();
    class_code = rec.value("class_code").toString();
    protocol = rec.value("protocol").toString();
    doctor = rec.value("doctor").toString();
    file_path = rec.value("file_path").toString();
    recording_flag = rec.value("recording_flag").toInt();
    video_flag = rec.value("video_flag").toInt();
    system = rec.value("recording_system").toString();
}

// TO DO - make Time_t --> QDateTime conversion part of QRecord constructor ?

QDateTime TimeT2QDateTime(time_t t){ // no need to be part of MainWindow
    // special implementation for XP and Win10 builds
#if QT_VERSION >= 0x050800
    QDateTime QnewTime = QDateTime::fromSecsSinceEpoch(t);
#else
    QDateTime QnewTime = QDateTime::fromTime_t(t);
#endif
    return QnewTime;
}

void QPatient::set_values(QRecord qrecord){
    id = qrecord.id;
    name = qrecord.name;
    sex = qrecord.sex;
    no = 1;
    last_record = qrecord.record_start;
    this->add_record(qrecord);
}

void QPatient::set_values_from_db(QSqlRecord rec){
    id = rec.value("id").toString();
    name = rec.value("name").toString();
    last_record = TimeT2QDateTime(rec.value("last_record").toInt());
    sex = rec.value("sex").toInt();
}

void QPatient::add_record(QRecord Qrecord){

    Qrecords_map.insert(Qrecord.file_name,Qrecord);

    // compares start of recordings and uses later
    if (Qrecord.record_start > last_record){
        last_record = Qrecord.record_start;
    };

    no = Qrecords_map.size();
};

QDataStream & operator<<(QDataStream & out, const QPatient & Qpatient)
{
    out << Qpatient.id << Qpatient.last_record << Qpatient.name << Qpatient.sex << (qint16)Qpatient.no << Qpatient.Qrecords_map;
    //qDebug() << "no out:" << Qpatient.no;
    return out;
}

QDataStream & operator<<(QDataStream & out, const QRecord & Qrecord)
{
    out << Qrecord.check_flag << Qrecord.class_code << Qrecord.file_name << Qrecord.file_path;
    out << (quint32)Qrecord.file_size << Qrecord.id << Qrecord.name << Qrecord.protocol << Qrecord.record_start;
    out << Qrecord.recording_flag << Qrecord.sex << Qrecord.video_flag << Qrecord.record_duration_s;

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
    in >> Qrecord.recording_flag >> Qrecord.sex >> Qrecord.video_flag >> Qrecord.record_duration_s;

    return in;
}
