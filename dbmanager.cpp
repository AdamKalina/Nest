#include "dbmanager.h"

//DbManager::DbManager(const QString& path)
//{
//    m_db = QSqlDatabase::addDatabase("QSQLITE");
//    m_db.setDatabaseName(path);

//    if (!m_db.open())
//    {
//        qDebug() << "Error: connection with database failed";
//    }
//    else
//    {
//        qDebug() << "Database: connection ok";
//    }
//}

DbManager::DbManager(){
}

bool DbManager::setPath(const QString& path){
    qDebug() << QSqlDatabase::drivers();


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

    if(!personExists(qrecord.id)){
        addPerson(qrecord);
    }else{
        // check if record exists and then update it or create new one - is it quicker?
        // adding new record - increment no in patient
        // updating record - without increment, update only - class_code, protocol, doctor, recording_flag, video_flag, num_pages INTEGER
        // is "no" really needed - maybe it would be better to just check number of successful queries
    }

    return true;
}

bool DbManager::createTablePatients()
{
    bool success = false;

    QSqlQuery query;
    query.prepare("CREATE TABLE IF NOT EXISTS patients(id TEXT PRIMARY KEY, name TEXT, sex TEXT, no INTEGER, last_record INTEGER);");

    if (!query.exec())
    {
        qDebug() << "Couldn't create the table 'patients': one might already exist.";
        success = false;
    }

    return success;
}

bool DbManager::createTableRecords()
{
    bool success = false;

    QSqlQuery query;
    query.prepare("CREATE TABLE IF NOT EXISTS records(file_name TEXT PRIMARY KEY, id TEXT, name TEXT, record_start INTEGER, sex TEXT,"
    "class_code TEXT, protocol TEXT, doctor TEXT, file_path TEXT, recording_flag INTEGER, video_flag INTEGER, num_pages INTEGER);");

    if (!query.exec())
    {
        qDebug() << "Couldn't create the table 'records': one might already exist.";
        success = false;
    }

    return success;
}

bool DbManager::addPerson(QRecord qrecord)
{
    bool success = false;
    QSqlQuery queryAdd;
    queryAdd.prepare("INSERT INTO patients (id, name, sex, no, last_record) VALUES (:id, :name, :sex, :no, :last_record)");
    queryAdd.bindValue(":id", qrecord.id);
    queryAdd.bindValue(":name", qrecord.name);
    queryAdd.bindValue(":sex", qrecord.sex);
    queryAdd.bindValue(":no", 0); // initiate as one and then increment when adding record
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

bool DbManager::removePerson(const QString& name)
{
    bool success = false;

    if (personExists(name))
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

bool DbManager::personExists(const QString& id) const
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
