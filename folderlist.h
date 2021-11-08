#ifndef FOLDERLIST_H
#define FOLDERLIST_H

#include "mainwindow.h"

class MainWindow;

class folderList : public QObject
{
    Q_OBJECT

public:
    folderList(QWidget *parent);
    MainWindow *mainwindow;

private:

    QDialog      *edit_folders_dialog,
    *dialog;

    QListWidget  *folder_path_list,
    *sfolder_path_list;

    QPushButton  *CloseButton,
    *button1,
    *button2,
    *button3;

    QListWidgetItem *listItem;

    int row;


private slots:

    //void rowClicked(QListWidgetItem *);
    //void adEntry();
    //void removeEntry();

};

#endif // FOLDERLIST_H
