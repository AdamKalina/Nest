#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QString>
#include <QDebug>
#include <QtSql>
#include "read_signal_file.h""

// based on https://github.com/katecpp/sql_with_qt - but added default constructor so it can be included in header of other class

class DbManager
{
public:

    DbManager();
    /**
     * @brief Constructor
     *
     * Constructor sets up connection with db and opens it
     * @param path - absolute path to db file
     */
    //DbManager(const QString& path);

    bool setPath(const QString& path);

    /**
     * @brief Destructor
     *
     * Close the db connection
     */
    ~DbManager();

    bool isOpen() const;

    /**
     * @brief Creates a new 'patients' table if it doesn't already exist
     * @return true - 'patients' table created successfully, false - table not created
     */
    bool createTablePatients();

    bool createTableRecords();

    /**
     * @brief Add person data to db
     * @param name - name of person to add
     * @return true - person added successfully, false - person not added
     */
    bool addPerson(QRecord qrecord);

    bool addRecord(QRecord qrecord);

    /**
     * @brief Remove person data from db
     * @param name - name of person to remove.
     * @return true - person removed successfully, false - person not removed
     */
    bool removePerson(const QString& name);

    /**
     * @brief Check if person of name "name" exists in db
     * @param name - name of person to check.
     * @return true - person exists, false - person does not exist
     */
    bool personExists(const QString& name) const;

    /**
     * @brief Print names of all persons in db
     */
    void printAllPersons() const;

    /**
     * @brief Remove all persons from db
     * @return true - all persons removed successfully, false - not removed
     */
    bool removeAllPersons();

private:
    QSqlDatabase m_db;
};

#endif // DBMANAGER_H
