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

    QPushButton  *CloseButton,
    *button1,
    *button2,
    *button3;

    QLineEdit *scanEdit,
    *controlEdit;

    int row;


private slots:

    //void rowClicked(QListWidgetItem *);
    //void adEntry();
    //void removeEntry();

};

#endif // EXTERNALPROGRAMLIST_H
