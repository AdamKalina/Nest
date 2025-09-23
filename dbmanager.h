#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QString>
#include <QDebug>
#include <QtSql>
#include "read_signal_file.h"

// based on https://github.com/katecpp/sql_with_qt - but added default constructor so it can be included in header of other class

class DbManager
{
public:

    DbManager();

    /**
     * @brief setPath = Constructor
     * Constructor sets up connection with db and opens it
     * @param path - absolute path to db file
     */
    bool setPath(const QString& path);

    /**
     * @brief Destructor
     *
     * Close the db connection
     */
    ~DbManager();

    /**
      * @brief Checks if the database is opened
      * @return true = db is opened, false = db is not opened
      */
    bool isOpen() const;

    /**
     * @brief Creates a new 'patients' table if it doesn't already exist
     * @return true - 'patients' table created successfully, false - table not created
     */
    bool createTablePatients();

    /**
     * @brief Creates a new index idx_last_record in TABLE 'patients' if it doesn't already exist
     * @return true - index created successfully, false - table not created
     */
    bool createIndexPatients();

    /**
     * @brief Creates a new 'records' table if it doesn't already exist
     * @return true - 'records' table created successfully, false - table not created
     */
    bool createTableRecords();

    /**
     * @brief Creates a new 'reports' table if it doesn't already exist
     * @return true - 'reports' table created successfully, false - table not created
     */
    bool createTableReports();

    /**
     * @brief Add person data to db patients
     * @param qrecord - qrecord with patients data
     * @return true - person added successfully, false - person not added
     */
    bool addPerson(QRecord qrecord);

    /**
     * @brief Add record data to db, logic inside checks if it already exists (--> updateRecord) or if it is new record (--> insertNewRecord)
     * @param qrecord - qrecord with record data
     * @return true - record added successfully, record - person not added
     */
    bool addRecord(QRecord qrecord);

    /**
     * @brief Insert new record to db
     * @param qrecord - qrecord with record data
     * @return true - new record inserted successfully, record - new record not inserted
     */
    bool insertNewRecord(QRecord qrecord);

    /**
     * @brief Update existing record
     * @param qrecord - qrecord with record data
     * @return true - record updated successfully, record - record not updated
     */
    bool updateRecord(QRecord qrecord);

    /**
     * @brief Finds patient for given records and checks if the newly inserted record is newer then last_record
     * @param qrecord - qrecord with record data
     * @return true - record updated successfully, record - record not updated
     */
    bool updatePatientLastRecord(QRecord qrecord);

    /**
     * @brief Remove person data from db
     * @param id - id of person to remove.
     * @return true - person removed successfully, false - person not removed
     */
    bool removePerson(const QString& id);

    /**
     * @brief Remove record data from db
     * @param file_name - file name of record to remove.
     * @return true - record removed successfully, false -  record not removed
     */
    bool removeRecord(const QString& file_name);

    /**
     * @brief Check if person of name "name" exists in db
     * @param name - name of person to check.
     * @return true - person exists, false - person does not exist
     */
    bool patientExists(const QString& name) const;

    /**
     * @brief Check if record of name "file_name" exists in db
     * @param file_name - name of record to check.
     * @return true - record exists, false - record does not exist
     */
    bool recordExists(const QString& file_name) const;

    /**
     * @brief Print names of all persons in db
     */
    void printAllPersons() const;

    /**
     * @brief Get patients ids from db where last_record > 2 years ago (hardcoded in the function)
     * @return Qvector<QString> of patients ids
     */
    QVector<QString> getPatientsIds();

    /**
     * @brief Get patients ids from db where last_record > months ago
     * @param months - number of months to look for in the past
     * @return Qvector<QString> of patients ids
     */
    QVector<QString> getPatientsIdsbyMonthsAgo(int months);

    /**
     * @brief Get ids of last X patients from db
     * @param no_of_patients - number of patients with EEG ordered by date of last EEG
     * @return Qvector<QString> of patients ids
     */
    QVector<QString> getLastXPatientsId(int no_of_patients);

    /**
     * @brief Get ids of next X patients from db on top of those already loaded
     * @param no_of_patients_loaded - number of patients already loaded order by date of last EEG
     * @return Qvector<QString> of patients ids
     */
    QVector<QString> getNextXPatientsId(int no_of_patients_loaded, int no_of_patients_to_load);

    /**
     * @brief Get patients ids from db where name/class_code/doctor LIKE query, wrapper function for getPatientsIdByTextNoteFromCol
     * @param query - QString inserted by user into filter line
     * @return Qvector<QString> of patients ids
     */
    QVector<QString> getPatientsIdbyTextNote(QString query);

    /**
     * @brief Get patients ids from db where field LIKE query
     * @param query - QString inserted by user into filter line, field - db col to search
     * @return Qvector<QString> of patients ids
     */
    QVector<QString> getPatientsIdByTextNoteFromCol(QString field, QString query);


    // function not used?
    bool selectPatient();

    /**
     * @brief Select patient from db by id with all its records
     * @param id - QString with patient's id
     * @return QPatient structure with all its qrecords
     */
    QPatient selectPatientbyIdWithRecords(QString id);

    /**
     * @brief Select patient from db by name with all its records
     * @param name - QString with patient's name
     * @return QPatient structure with all its qrecords
     */
    QPatient selectPatientbyNameWithRecords(QString name);

    /**
     * @brief Select report from "reports" using file_path
     * @param file_path - QString with path to file
     * @return record structure
     */

    QSqlRecord getReportByFilePath(QString file_path);

    /**
     * @brief Select report from "reports" using file_id
     * @param file_path - QString with file id
     * @return record structure
     */

    QSqlRecord getReportByFileId(QString file_id);

    /**
     * @brief Remove all persons from db
     * @return true - all persons removed successfully, false - not removed
     */

    bool removeAllPersons();

private:
    QSqlDatabase m_db;
};

#endif // DBMANAGER_H
