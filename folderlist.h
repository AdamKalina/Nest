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

    QListWidget  *dfolder_path_list,
    *sfolder_path_list;

    QPushButton  *CloseButton,
    *add_static_button,
    *refresh_sel_static_button,
    *add_dynamic_button,
    *button1,
    *button2,
    *button3,
    *button4;

    QListWidgetItem *currentItem;

    QLabel *staticLabel,
    *dynamicLabel,
    *helpLabel;

    QMessageBox *duplicate_msgBox;

    int row;
    bool currentMode;
    void rowClicked(QListWidgetItem *, bool dynamic);
    void add_folder(bool dynamic);


private slots:
    void rowClickedDynamic(QListWidgetItem *);
    void rowClickedStatic(QListWidgetItem *);
    void adEntry();
    void removeEntry();
    void refreshEntry();
    void add_folder_dynamic();
    void add_folder_static();
    void refresh_sel_static();

};

#endif // FOLDERLIST_H
