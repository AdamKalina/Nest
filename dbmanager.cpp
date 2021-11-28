#include "dbmanager.h"

DbManager::DbManager(){
}

bool DbManager::setPath(const QString& path){

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(path);

    if (!m_db.open())
    {
        qDebug() << "Error: connection with database failed";
    }
    else
    {
        qDebug() << "Database: connection ok";
    }
    return true;
}

DbManager::~DbManager()
{
    if (m_db.isOpen())
    {
        m_db.close();
    }
}

bool DbManager::isOpen() const
{
    return m_db.isOpen();
}

bool DbManager::addRecord(QRecord qrecord){

    // TO DO - redo this function
    // 1) check if record exists
    //  1a) record exists --> update record
    //  1b) record does not exist --> insert as new record AND 2) check if patient exists
    // 2a) patient exists --> update last_record
    // 2b) patient does not exist --> create new patient

    if(recordExists(qrecord.file_name)){ // check if record exists
        updateRecord(qrecord); // updating record - only class_code, protocol, doctor, recording_flag, video_flag, num_pages
    }else{
        insertNewRecord(qrecord); // insert as new and update patient
        if(!patientExists(qrecord.id)){
            addPerson(qrecord);
        }else{
            updatePatientLastRecord(qrecord);
        }
    }

    //    if(!personExists(qrecord.id)){
    //        addPerson(qrecord);
    //        insertNewRecord(qrecord,0); // no need to update patient since this is the first entry
    //    }else{
    //        if(!recordExists(qrecord.file_name)){ // check if record exists
    //            insertNewRecord(qrecord,1); // insert as new and update patient
    //        }else{
    //            updateRecord(qrecord); // updating record - only class_code, protocol, doctor, recording_flag, video_flag, num_pages
    //        }
    //    }

    return true;
}

bool DbManager::insertNewRecord(QRecord qrecord){
    bool success = false;

    // insert into TABLE records
    QSqlQuery query;
    query.prepare("INSERT INTO records (file_name, id, name, record_start, sex, class_code, protocol, doctor, file_path, recording_flag, video_flag, num_pages)"
    "VALUES (:file_name, :id, :name , :record_start, :sex, :class_code, :protocol, :doctor, :file_path, :recording_flag, :video_flag, :num_pages)");
    query.bindValue(":file_name", qrecord.file_name);
    query.bindValue(":id", qrecord.id);
    query.bindValue(":name", qrecord.name);
    query.bindValue(":record_start", qrecord.record_start);
    query.bindValue(":sex", qrecord.sex);
    query.bindValue(":class_code", qrecord.class_code);
    query.bindValue(":protocol",qrecord.protocol);
    query.bindValue(":doctor", qrecord.doctor);
    query.bindValue(":file_path",qrecord.file_path);
    query.bindValue(":recording_flag",qrecord.recording_flag);
    query.bindValue(":video_flag",qrecord.video_flag);
    query.bindValue(":num_pages",qrecord.video_flag);

    if (!query.exec())
    {
        qDebug() << "Couldn't INSERT INTO the table 'records' " << query.lastError();
        success = false;
    }

    return success;
}

bool DbManager::updateRecord(QRecord qrecord){
    bool success = true;

    // UPDATE TABLE records
    QSqlQuery query;
    query.prepare("UPDATE records SET class_code=:class_code, protocol=:protocol, doctor=:doctor, file_path=:file_path, recording_flag=:recording_flag, video_flag=:video_flag, num_pages=:num_pages WHERE file_name =:file_name");
    query.bindValue(":class_code", qrecord.class_code);
    query.bindValue(":protocol",qrecord.protocol);
    query.bindValue(":doctor", qrecord.doctor);
    query.bindValue(":file_path",qrecord.file_path);
    query.bindValue(":recording_flag",qrecord.recording_flag);
    query.bindValue(":video_flag",qrecord.video_flag);
    query.bindValue(":num_pages",qrecord.num_pages);
    query.bindValue(":file_name",qrecord.file_name);

    if (!query.exec())
    {
        qDebug() << "Couldn't UPDATE the table 'records' " << query.lastError();
        success = false;
    }

    return success;
}

bool DbManager::updatePatientLastRecord(QRecord qrecord){
    QSqlQuery selectQuery;
    selectQuery.prepare("SELECT last_record FROM patients WHERE id = (:id)");
    selectQuery.bindValue(":id", qrecord.id);

    if (!selectQuery.exec())
    {
        qDebug() << "Couldn't find ID in the table 'patients' " << selectQuery.lastError();
        return false;
    }
    selectQuery.next();
    time_t old_time = selectQuery.value("last_record").toInt();

    if(old_time < qrecord.record_start){
        QSqlQuery updateQuery;
        updateQuery.prepare("UPDATE patients SET last_record=:last_record WHERE id =:id");
        updateQuery.bindValue(":last_record",qrecord.record_start);
        updateQuery.bindValue(":id",qrecord.id);

        if (!updateQuery.exec())
        {
            qDebug() << "Couldn't update last_record in the table 'patients'" << updateQuery.lastError();
            return false;
        }
        else{
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

    if (!query.exec())
    {
        qDebug() << "Couldn't create the table 'patients': one might already exist. " << query.lastError();
        success = false;
    }

    return success;
}

bool DbManager::createIndexPatients(){

    bool success = true;

    QSqlQuery query;
    query.prepare("CREATE INDEX IF NOT EXISTS idx_last_record ON patients (last_record);");

    if (!query.exec())
    {
        qDebug() << "Couldn't create index in the table 'patients'";
        success = false;
    }

    return success;
}

bool DbManager::createTableRecords(){
    bool success = true;

    QSqlQuery query;
    query.prepare("CREATE TABLE IF NOT EXISTS records(file_name TEXT PRIMARY KEY, id TEXT, name TEXT, record_start INTEGER, sex INTEGER,"
    "class_code TEXT, protocol TEXT, doctor TEXT, file_path TEXT, recording_flag INTEGER, video_flag INTEGER, num_pages INTEGER);");

    if (!query.exec())
    {
        qDebug() << "Couldn't create the table 'records': one might already exist.";
        success = false;
    }

    return success;
}

bool DbManager::addPerson(QRecord qrecord){
    bool success = false;
    QSqlQuery queryAdd;
    queryAdd.prepare("INSERT INTO patients (id, name, sex, last_record) VALUES (:id, :name, :sex, :last_record)");
    queryAdd.bindValue(":id", qrecord.id);
    queryAdd.bindValue(":name", qrecord.name);
    queryAdd.bindValue(":sex", qrecord.sex);
    queryAdd.bindValue(":last_record", qrecord.record_start);

    if(queryAdd.exec())
    {
        success = true;
    }
    else
    {
        qDebug() << "add person failed: " << queryAdd.lastError();
    }
    return success;
}


// not my function
bool DbManager::removePerson(const QString& name)
{
    bool success = false;

    if (patientExists(name))
    {
        QSqlQuery queryDelete;
        queryDelete.prepare("DELETE FROM patients WHERE name = (:name)");
        queryDelete.bindValue(":name", name);
        success = queryDelete.exec();

        if(!success)
        {
            qDebug() << "remove person failed: " << queryDelete.lastError();
        }
    }
    else
    {
        qDebug() << "remove person failed: person doesnt exist";
    }

    return success;
}

// not my function
void DbManager::printAllPersons() const
{
    qDebug() << "Patients in db:";
    //QSqlQuery query("SELECT * FROM people");
    QSqlQuery query("SELECT * FROM patients");
    int idName = query.record().indexOf("name");
    while (query.next())
    {
        QString name = query.value(idName).toString();
        qDebug() << "===" << name;
    }
}

bool DbManager::patientExists(const QString& id) const
{
    bool exists = false;

    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT id FROM patients WHERE id = (:id)");
    checkQuery.bindValue(":id", id);

    if (checkQuery.exec())
    {
        if (checkQuery.next())
        {
            exists = true;
        }
    }
    else
    {
        qDebug() << "person exists failed: " << checkQuery.lastError();
    }

    return exists;
}

bool DbManager::recordExists(const QString& file_name) const
{
    bool exists = false;

    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT file_name FROM records WHERE file_name = (:file_name)");
    checkQuery.bindValue(":file_name", file_name);

    if (checkQuery.exec())
    {
        if (checkQuery.next())
        {
            exists = true;
        }
    }
    else
    {
        qDebug() << "record exists failed: " << checkQuery.lastError();
    }

    return exists;
}

// TO DO - generic version of this function
QVector<QString> DbManager::getPatientsIds(){

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

    int longAgo = QDateTime::currentDateTimeUtc().addMonths(-months).toTime_t();
    //qDebug()  << twoYearsAgo;

    QVector<QString> qpatientIds;
    QSqlQuery selectQuery;
    selectQuery.prepare("SELECT id FROM patients WHERE last_record > :date");
    selectQuery.bindValue(":date",longAgo);
    //selectQuery.prepare("SELECT id FROM patients");

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


bool DbManager::selectPatient(){
    bool success = false;

    QSqlQuery selectQuery;
    selectQuery.prepare("SELECT id, name, last_record, sex FROM patients");
    selectQuery.exec();

    while (selectQuery.next())
    {
        QPatient qpatient;
        qpatient.set_values_from_db(selectQuery.record());

        qDebug() << "===" << qpatient.id << "===" << qpatient.name << "===" << QDateTime::fromTime_t(qpatient.last_record);

        QSqlQuery selectRecordQuery;
        selectRecordQuery.prepare("SELECT file_name, id, name, record_start, sex, class_code, protocol, doctor, file_path, recording_flag, video_flag, num_pages FROM records WHERE id = (:id)");
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

    QPatient qpatient;

    QSqlQuery selectRecordQuery;
    selectRecordQuery.prepare("SELECT file_name, id, name, record_start, sex, class_code, protocol, doctor, file_path, recording_flag, video_flag, num_pages FROM records WHERE id = (:id)");
    selectRecordQuery.bindValue(":id",id);
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
        qDebug() << "Problem with selecting record by ID" << selectRecordQuery.lastError();
    }

    return qpatient;
}

QPatient DbManager::selectPatientbyNameWithRecords(QString name){

    QPatient qpatient;

    QSqlQuery selectRecordQuery;
    selectRecordQuery.prepare("SELECT file_name, id, name, record_start, sex, class_code, protocol, doctor, file_path, recording_flag, video_flag, num_pages FROM records WHERE name = (:name)");
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

    if (removeQuery.exec())
    {
        success = true;
    }
    else
    {
        qDebug() << "remove all persons failed: " << removeQuery.lastError();
    }

    return success;
}
