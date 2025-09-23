#ifndef READ_NICOLET_DB_H
#define READ_NICOLET_DB_H

#include <QString>
#include <QDebug>
#include <QtSql>
#include <QSqlDatabase>
#include <QSqlQuery>

class read_nicolet_db
{
public:
    read_nicolet_db();
    bool connect(const QString& path);
    ~read_nicolet_db();
    bool isOpen() const;
    bool getTableStructure(const QString& table);
    bool getDbStructure();
    bool listAllPatients();
    bool getPatientInfoById(const QString& id);
    QStringList getPatientById(const QString& id);
    QString getStrStudyNo(const QString& guid);
    bool getStudyById(const QString& id);
    bool listAllStudies();
    void closeDb();

private:
    QSqlDatabase nicolet_db;
};

#endif // READ_NICOLET_DB_H
