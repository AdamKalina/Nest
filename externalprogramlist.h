#ifndef EXTERNALPROGRAMLIST_H
#define EXTERNALPROGRAMLIST_H

#include "mainwindow.h"

class MainWindow;

class externalprogramlist : public QObject
{
    Q_OBJECT

public:
    externalprogramlist(QWidget *parent);
    MainWindow *mainwindow;

private:

    QDialog      *edit_program_dialog,
    *dialog;

    QPushButton  *cancelButton,
    *saveButton,
    *AddReaderButton,
    *AddControlButton;

    QLineEdit *readerEdit,
    *controlEdit;
    void add_program(QString program);


private slots:
    void add_reader();
    void add_control();
    void saveAndClose();

};

#endif // EXTERNALPROGRAMLIST_H
