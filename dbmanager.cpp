#include "dbmanager.h"

DbManager::DbManager(){
}

bool DbManager::setPath(const QString& path){
    qDebug() << "DbManager::setPath";

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(path);

    if (!m_db.open())
    {
        qDebug() << "Error: connection with database failed";
    }else{
        qDebug() << "Database: connection ok";
    }
    return true;
}

DbManager::~DbManager(){
    qDebug() << "DbManager::~DbManager";
    if (m_db.isOpen()){
        m_db.close();
    }
}

bool DbManager::isOpen() const{
    return m_db.isOpen();
}

bool DbManager::addRecord(QRecord qrecord){
    //qDebug() << "DbManager::addRecord";

    if(recordExists(qrecord.file_name)){ // check if record exists
        updateRecord(qrecord); // updating record - only class_code, protocol, doctor, file_path, recording_flag, video_flag, file_length
    }else{
        insertNewRecord(qrecord); // insert as new and update patient
        if(!patientExists(qrecord.id)){
            addPerson(qrecord);
        }else{
            updatePatientLastRecord(qrecord);
        }
    }
    return true;
}

bool DbManager::insertNewRecord(QRecord qrecord){
    //qDebug() << "DbManager::insertNewRecord";
    bool success = false;

    // insert into TABLE records
    QSqlQuery query;
    query.prepare("INSERT INTO records (file_name, id, name, record_start, record_duration_s, sex, class_code, protocol, doctor, file_path, recording_flag, video_flag, recording_system)"
    "VALUES (:file_name, :id, :name , :record_start, :record_duration_s, :sex, :class_code, :protocol, :doctor, :file_path, :recording_flag, :video_flag, :recording_system)");
    query.bindValue(":file_name", qrecord.file_name);
    query.bindValue(":id", qrecord.id);
    query.bindValue(":name", qrecord.name);
    query.bindValue(":record_start", qrecord.record_start.toTime_t());
    query.bindValue(":sex", qrecord.sex);
    query.bindValue(":class_code", qrecord.class_code);
    query.bindValue(":protocol",qrecord.protocol);
    query.bindValue(":doctor", qrecord.doctor);
    query.bindValue(":file_path",qrecord.file_path);
    query.bindValue(":recording_flag",qrecord.recording_flag);
    query.bindValue(":video_flag",qrecord.video_flag);
    query.bindValue(":record_duration_s",qrecord.record_duration_s);
    query.bindValue(":recording_system", qrecord.system);

    if (!query.exec()){
        qDebug() << "Couldn't INSERT INTO the table 'records' " << query.lastError();
        success = false;
    }

    return success;
}

bool DbManager::updateRecord(QRecord qrecord){
    // it also allows for update of name and id because sometimes technicians make mistake in that
    bool success = true;
    //qDebug() << "DbManager::updateRecord";

    // UPDATE TABLE records
    QSqlQuery query;
    query.prepare("UPDATE records SET id=:id, name=:name, record_start=:record_start, class_code=:class_code, protocol=:protocol, doctor=:doctor, file_path=:file_path, recording_flag=:recording_flag, video_flag=:video_flag, record_duration_s=:record_duration_s WHERE file_name =:file_name");
    query.bindValue(":id",qrecord.id);
    query.bindValue(":name",qrecord.name);
    query.bindValue(":record_start", qrecord.record_start.toTime_t());
    query.bindValue(":record_duration_s",qrecord.record_duration_s);
    query.bindValue(":class_code", qrecord.class_code);
    query.bindValue(":protocol",qrecord.protocol);
    query.bindValue(":doctor", qrecord.doctor);
    query.bindValue(":file_path",qrecord.file_path);
    query.bindValue(":recording_flag",qrecord.recording_flag);
    query.bindValue(":video_flag",qrecord.video_flag);
    query.bindValue(":file_name",qrecord.file_name);

    if (!query.exec()){
        qDebug() << "Couldn't UPDATE the table 'records' " << query.lastError();
        success = false;
    }

    return success;
}

bool DbManager::updatePatientLastRecord(QRecord qrecord){
    qDebug() << "updatePatientLastRecord";
    QSqlQuery selectQuery;
    selectQuery.prepare("SELECT last_record FROM patients WHERE id = (:id)");
    selectQuery.bindValue(":id", qrecord.id);

    if (!selectQuery.exec()){
        qDebug() << "Couldn't find ID in the table 'patients' " << selectQuery.lastError();
        return false;
    }
    selectQuery.next();
    time_t old_time = selectQuery.value("last_record").toInt();

    if(old_time < qrecord.record_start.toTime_t()){
        QSqlQuery updateQuery;
        updateQuery.prepare("UPDATE patients SET last_record=:last_record WHERE id =:id");
        updateQuery.bindValue(":last_record",qrecord.record_start.toTime_t());
        updateQuery.bindValue(":id",qrecord.id);

        if (!updateQuery.exec()){
            qDebug() << "Couldn't update last_record in the table 'patients'" << updateQuery.lastError();
            return false;
        }else{
            qDebug() << "Updated last_record in patients";
        }
    }else{
        qDebug() << "no need to update last_record";
    }
    return true;
}

bool DbManager::createTablePatients(){
    bool success = true;

    QSqlQuery query;
    query.prepare("CREATE TABLE IF NOT EXISTS patients(id TEXT PRIMARY KEY, name TEXT, sex INTEGER, last_record INTEGER);");

    if (!query.exec()){
        qDebug() << "Couldn't create the table 'patients': one might already exist. " << query.lastError();
        success = false;
    }

    return success;
}

bool DbManager::createIndexPatients(){

    bool success = true;

    QSqlQuery query;
    query.prepare("CREATE INDEX IF NOT EXISTS idx_last_record ON patients (last_record);");

    if (!query.exec()){
        qDebug() << "Couldn't create index in the table 'patients'";
        success = false;
    }

    return success;
}

bool DbManager::createTableRecords(){
    bool success = true;

    QSqlQuery query;
    query.prepare("CREATE TABLE IF NOT EXISTS records(file_name TEXT PRIMARY KEY, id TEXT, name TEXT, record_start INTEGER, record_duration_s INTEGER, sex INTEGER,"
    "class_code TEXT, protocol TEXT, doctor TEXT, file_path TEXT, recording_flag INTEGER, video_flag INTEGER, recording_system TEXT);");

    if (!query.exec()){
        qDebug() << "Couldn't create the table 'records': one might already exist.";
        success = false;
    }

    return success;
}

bool DbManager::addPerson(QRecord qrecord){
    //qDebug() << "DbManager::addPerson";
    bool success = false;
    QSqlQuery queryAdd;
    queryAdd.prepare("INSERT INTO patients (id, name, sex, last_record) VALUES (:id, :name, :sex, :last_record)");
    queryAdd.bindValue(":id", qrecord.id);
    queryAdd.bindValue(":name", qrecord.name);
    queryAdd.bindValue(":sex", qrecord.sex);
    queryAdd.bindValue(":last_record", qrecord.record_start.toTime_t());

    if(queryAdd.exec()){
        success = true;
    }else{
        qDebug() << "add person failed: " << queryAdd.lastError();
    }
    return success;
}


bool DbManager::removePerson(const QString& id){
    bool success = false;

    if (patientExists(id)){
        QSqlQuery queryDelete;
        queryDelete.prepare("DELETE FROM patients WHERE id = (:id)");
        queryDelete.bindValue(":id", id);
        success = queryDelete.exec();

        if(!success){
            qDebug() << "remove person failed: " << queryDelete.lastError();
        }
    }else{
        qDebug() << "remove person failed: person doesn't exist";
    }

    return success;
}

bool DbManager::removeRecord(const QString& file_name){
    bool success = false;

    if (recordExists(file_name)){
        QSqlQuery queryDelete;
        queryDelete.prepare("DELETE FROM records WHERE file_name = (:file_name)");
        queryDelete.bindValue(":file_name", file_name);
        success = queryDelete.exec();

        if(!success){
            qDebug() << "remove record failed: " << queryDelete.lastError();
        }
    }else{
        qDebug() << "remove record failed: record doesn't exist";
    }

    return success;
}

// not my function
void DbManager::printAllPersons() const{
    qDebug() << "Patients in db:";
    //QSqlQuery query("SELECT * FROM people");
    QSqlQuery query("SELECT * FROM patients");
    int idName = query.record().indexOf("name");
    while (query.next()){
        QString name = query.value(idName).toString();
        qDebug() << "===" << name;
    }
}

bool DbManager::patientExists(const QString& id) const{
    qDebug() << "DbManager::patientExists";
    bool exists = false;

    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT id FROM patients WHERE id = (:id)");
    checkQuery.bindValue(":id", id);

    if (checkQuery.exec()){
        if (checkQuery.next()){
            exists = true;
        }
    }else{
        qDebug() << "person exists failed: " << checkQuery.lastError();
    }

    return exists;
}

bool DbManager::recordExists(const QString& file_name) const{
    //qDebug() << "DbManager::recordExists";
    bool exists = false;

    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT file_name FROM records WHERE file_name = (:file_name)");
    checkQuery.bindValue(":file_name", file_name);

    if (checkQuery.exec()){
        if (checkQuery.next()){
            exists = true;
        }
    }else{
        qDebug() << "record exists failed: " << checkQuery.lastError();
    }

    return exists;
}

// TO DO - generic version of this function
QVector<QString> DbManager::getPatientsIds(){
    //qDebug() << "DbManager::getPatientsIds()";

    int twoYearsAgo = QDateTime::currentDateTimeUtc().addYears(-2).toTime_t();
    //qDebug()  << twoYearsAgo;

    QVector<QString> qpatientIds;
    QSqlQuery selectQuery;
    selectQuery.prepare("SELECT id FROM patients WHERE last_record > :date");
    selectQuery.bindValue(":date",twoYearsAgo);
    //selectQuery.prepare("SELECT id FROM patients");

    if(selectQuery.exec()){
        while(selectQuery.next()){
            qpatientIds.append(selectQuery.value("id").toString());
            //qDebug() << selectQuery.value("id").toString();
        }

    }else{
        qDebug() << "problem with selecting patients by ID" << selectQuery.lastError();
    }

    return qpatientIds;
}

QVector<QString> DbManager::getPatientsIdsbyMonthsAgo(int months){
    //qDebug() << "DbManager::getPatientsIdsbyMonthsAgo";


    int longAgo = QDateTime::currentDateTimeUtc().addMonths(-months).toTime_t();
    //qDebug()  << twoYearsAgo;

    QVector<QString> qpatientIds;
    QSqlQuery selectQuery;
    selectQuery.prepare("SELECT id FROM patients WHERE last_record > :date");
    selectQuery.bindValue(":date",longAgo);

    if(selectQuery.exec()){
        while(selectQuery.next()){
            qpatientIds.append(selectQuery.value("id").toString());
            //qDebug() << selectQuery.value("id").toString();
        }

    }else{
        qDebug() << "problem with selecting patients by ID and months ago" << selectQuery.lastError();
    }

    return qpatientIds;
}

QVector<QString> DbManager::getPatientsIdbyTextNote(QString query){
    //qDebug() << "DbManager::getPatientsIdbyTextNote";
    QVector<QString> qpatientIds;
    QStringList cols;
    cols << "name" << "class_code" << "doctor";
    //qDebug() << cols;

    //now run "like" sql query in given cols
    for (int i = 0; i < cols.size(); ++i){
        qpatientIds.append(getPatientsIdByTextNoteFromCol(cols.at(i), query));
    }


    // unique only? but it is already checked in mainwindow

    return qpatientIds;
}

QVector<QString> DbManager::getPatientsIdByTextNoteFromCol(QString col, QString query){
    //qDebug() << "DbManager::getPatientsIdByTextNoteFromCol";
    //qDebug() << "now searching " << col << " for " << query;

    QString query_string = "SELECT id FROM records WHERE ";
    query_string += col;
    query_string += " LIKE :query";

    //qDebug() << query_string;

    QVector<QString> qpatientIds;
    QSqlQuery selectQuery;
    selectQuery.prepare(query_string);
    selectQuery.bindValue(":query", ("%" + query + "%"));
    if(selectQuery.exec()){
        while(selectQuery.next()){
            qpatientIds.append(selectQuery.value("id").toString());
            //qDebug() << selectQuery.value("id").toString();
        }
    }else{
        qDebug() << "problem with selecting records in field: " << col << selectQuery.lastError();
    }
    return qpatientIds;
}


bool DbManager::selectPatient(){
    qDebug() << "DbManager::selectPatient";
    bool success = false;

    QSqlQuery selectQuery;
    selectQuery.prepare("SELECT id, name, last_record, sex FROM patients");
    selectQuery.exec();

    while (selectQuery.next()){
        QPatient qpatient;
        qpatient.set_values_from_db(selectQuery.record());

        qDebug() << "===" << qpatient.id << "===" << qpatient.name << "===" << qpatient.last_record;

        QSqlQuery selectRecordQuery;
        selectRecordQuery.prepare("SELECT file_name, id, name, record_start, record_duration_s, sex, class_code, protocol, doctor, file_path, recording_flag, video_flag, FROM records WHERE id = (:id)");
        selectRecordQuery.bindValue(":id",qpatient.id);
        selectRecordQuery.exec();

        if(selectRecordQuery.exec()){
            while (selectRecordQuery.next()){

                QRecord qrecord;
                qrecord.set_values_from_db(selectRecordQuery.record());
                qpatient.add_record(qrecord);

                qDebug() << "======" << qrecord.file_name;
            }
            qDebug() << "======" << qpatient.no;
        }else{
            qDebug() << "Problem with selecting record by ID" << selectRecordQuery.lastError();
        }

    }

    return success;
}

QPatient DbManager::selectPatientbyIdWithRecords(QString id){
    qDebug() << "DbManager::selectPatientbyIdWithRecords";

    QPatient qpatient;

    QSqlQuery selectRecordQuery;
    selectRecordQuery.prepare("SELECT file_name, id, name, record_start, record_duration_s, sex, class_code, protocol, doctor, file_path, recording_flag, video_flag, recording_system FROM records WHERE id = (:id)");
    selectRecordQuery.bindValue(":id",id);
    selectRecordQuery.exec();

    if(selectRecordQuery.exec()){
        while (selectRecordQuery.next()){

            QRecord qrecord;
            qrecord.set_values_from_db(selectRecordQuery.record());

            if(selectRecordQuery.at() == 0){ // first records - set qpatient values
                //qDebug() << "first";
                qpatient.set_values(qrecord);
                //qDebug() << qpatient.name;
            }else{
                qpatient.add_record(qrecord);
            }
            //qDebug() << "======" << qrecord.file_name;
        }
        //qDebug() << "======" << qpatient.no;
    }else{
        qDebug() << "Problem with selecting record by ID" << selectRecordQuery.lastError();
    }

    return qpatient;
}

QPatient DbManager::selectPatientbyNameWithRecords(QString name){
    qDebug() << "selectPatientbyNameWithRecords";
    QPatient qpatient;

    QSqlQuery selectRecordQuery;
    selectRecordQuery.prepare("SELECT file_name, id, name, record_start, record_duration_s, sex, class_code, protocol, doctor, file_path, recording_flag, video_flag FROM records WHERE name = (:name)");
    selectRecordQuery.bindValue(":name",name);
    selectRecordQuery.exec();

    if(selectRecordQuery.exec()){
        while (selectRecordQuery.next()){

            QRecord qrecord;
            qrecord.set_values_from_db(selectRecordQuery.record());

            if(selectRecordQuery.at() == 0){
                //qDebug() << "first";
                qpatient.set_values(qrecord);
                //qDebug() << qpatient.name;
            }else{
                qpatient.add_record(qrecord);
            }
            //qDebug() << "======" << qrecord.file_name;
        }
        //qDebug() << "======" << qpatient.no;
    }else{
        qDebug() << "Problem with selecting record by name" << selectRecordQuery.lastError();
    }

    return qpatient;
}

// not my function
bool DbManager::removeAllPersons()
{
    bool success = false;

    QSqlQuery removeQuery;
    removeQuery.prepare("DELETE FROM patients");

    if (removeQuery.exec()){
        success = true;
    }else{
        qDebug() << "remove all persons failed: " << removeQuery.lastError();
    }

    return success;
}
