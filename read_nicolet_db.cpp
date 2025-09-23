#include "read_nicolet_db.h"

read_nicolet_db::read_nicolet_db()
{

}

bool read_nicolet_db::connect(const QString& path){
    qDebug() << "read_nicolet_db::connect";

    nicolet_db = QSqlDatabase::addDatabase("QODBC3","nicolet_db"); // the connection name is needed, otherwise it defaults to defaultConnection
    qDebug() << "QSqlDatabase::isDriverAvailable(QODBC3):" << QSqlDatabase::isDriverAvailable("QODBC3");
    qDebug() << "path to MDB file" << path;
    //nicolet_db.setDatabaseName("DRIVER={Microsoft Access Driver (*.mdb,*.accdb)};DSN='';DBQ=d:/Dropbox/Scripts/Cpp/NicOne db/Záloha z TAUG.MDB;");
    //nicolet_db.setDatabaseName("DRIVER={Microsoft Access Driver (*.mdb, *.accdb)};FIL={MS Access};DBQ=d:/Dropbox/Scripts/Cpp/TAUG.MDB");
    nicolet_db.setDatabaseName("DRIVER={Microsoft Access Driver (*.mdb, *.accdb)};FIL={MS Access};DBQ="+path+"");
    qDebug() << "QODBC3 connectionName" << nicolet_db.connectionName();
    bool success = nicolet_db.open(); // tady to hází C++ exception, kterému nerozumím - at 0x7663b552, code: 0xe06d7363

    if (!success){
        qDebug() << "Error: connection with database failed";
        qDebug() << nicolet_db.lastError().text();
    }else{
        qDebug() << "QODBC3 database: connection ok";
    }
    return success;
}

read_nicolet_db::~read_nicolet_db(){
    qDebug() << "read_nicolet_db::~read_nicolet_db";
    if (nicolet_db.isOpen()){
        nicolet_db.close();
    }
}

bool read_nicolet_db::isOpen() const{
    return nicolet_db.isOpen();
}

bool read_nicolet_db::getDbStructure(){
    qDebug() << "read_nicolet_db::getDbStructure()";
    bool success = false;
    qDebug() << nicolet_db.QSqlDatabase::tables();
    qDebug() << nicolet_db.lastError().text();
    return success;
}

bool read_nicolet_db::getTableStructure(const QString& table){
    bool success = false;
    // get the list of names of fields in the table
    QSqlRecord record = nicolet_db.record(table); //get the record of the certain table
    int n = record.count();
    qDebug() << "no of fields: " << n;
    for(int i = 0; i < n; i++)
    {
        QString strField = record.fieldName(i);
        qDebug() << strField;
    }
    qDebug() << nicolet_db.lastError().text();
    return success;
}

bool read_nicolet_db::listAllPatients(){
    bool success = false;
    QSqlQuery query("SELECT * FROM tblPatient");
    int idName = query.record().indexOf("strLastName");
    int idID = query.record().indexOf("strAlternativeID");
    while (query.next()){
        QString name = query.value(idName).toString();
        QString id = query.value(idID).toString();
        qDebug() << name << " - " << id;
    }

    //    if (!query.exec()){
    //        qDebug() << "Couldn't READ from the table 'tblPatient' " << query.lastError();
    //        success = false;
    //    }
    return success;
}

bool read_nicolet_db::listAllStudies(){
    qDebug() << "listAllStudies()";
    bool success = false;
    QSqlQuery query("SELECT * FROM tblStudy");
    int idName = query.record().indexOf("guidStudyID");
    int idID = query.record().indexOf("strStudyNo");
    while (query.next()){
        QString name = query.value(idName).toString();
        QString id = query.value(idID).toString();
        qDebug() << name << " - " << id;
    }

    //    if (!query.exec()){
    //        qDebug() << "Couldn't READ from the table 'tblPatient' " << query.lastError();
    //        success = false;
    //    }
    return success;
}

bool read_nicolet_db::getPatientInfoById(const QString& id){
    qDebug() << "getting id: " << id;
    bool success = false;
    QSqlQuery query("SELECT * FROM tblPatient WHERE strAlternativeID = '"+id+"'");
    QSqlRecord record = nicolet_db.record("tblPatient"); //get the record of the certain table
    int rc = query.record().count();
    qDebug() << "no if fields: " << rc;
    int idName = query.record().indexOf("strLastName");
    while (query.next()){
        QString name = query.value(idName).toString();
        qDebug() << "===========================" << name << "===========================";
        for(int i = 0; i < rc; i++)
        {
            QString strField = query.value(i).toString();
            //qDebug() << record.fieldName(i) << ": " <<strField;
            if (strField != ""){
                qDebug() << record.fieldName(i) << "\t" << strField;
            }
        }
    }
    //    if (!query.exec()){
    //        qDebug() << "Couldn't READ from the table 'tblPatient' " << query.lastError();
    //        success = false;
    //    }
    //qDebug() << nicolet_db.drivers();
    qDebug() << nicolet_db.lastError();
    return success;
}

QStringList read_nicolet_db::getPatientById(const QString &id){

    qDebug() << "getting guidPatientID for id: " << id;
    QStringList ids;

    QString idd = id;

    QSqlTableModel model(nullptr, nicolet_db);
    model.setTable("tblPatient");
    model.setFilter(QString("strAlternativeID = '%1'").arg(idd.replace("'", "''")));
    model.select();

    for (int i = 0; i < model.rowCount(); i++){
        ids << model.record(0).value("guidPatientID").toString();
    }

    return ids;
}

QString read_nicolet_db::getStrStudyNo(const QString &guid){
    QString idd = guid;
    QSqlTableModel model(nullptr, nicolet_db);
    model.setTable("tblStudy");
    model.setFilter(QString("guidStudyID = '%1'").arg(idd.replace("'", "''")));
    model.select();
    return model.record(0).value("strStudyNo").toString(); // I expect just one result
}

bool read_nicolet_db::getStudyById(const QString& id){
    qDebug() << "getting study id: " << id;
    bool success = false;
    QSqlQuery query("SELECT * FROM tblStudy WHERE guidStudyID = '"+id+"'");
    QSqlRecord record = nicolet_db.record("tblStudy"); //get the record of the certain table
    int rc = query.record().count();
    qDebug() << "no of fields: " << rc;
    int idName = query.record().indexOf("guidStudyID");
    while (query.next()){
        QString name = query.value(idName).toString();
        qDebug() << "===========================" << name << "===========================";
        for(int i = 0; i < rc; i++)
        {
            QString strField = query.value(i).toString();
            //qDebug() << record.fieldName(i) << ": " <<strField;
            if (strField != ""){
                qDebug() << record.fieldName(i) << "\t" << strField;
            }
        }
    }
    //    if (!query.exec()){
    //        qDebug() << "Couldn't READ from the table 'tblPatient' " << query.lastError();
    //        success = false;
    //    }
    //qDebug() << nicolet_db.drivers();
    qDebug() << nicolet_db.lastError();
    return success;
}
