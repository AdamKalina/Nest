#ifndef FOLDERLIST_H
#define FOLDERLIST_H

#include "mainwindow.h"

class MainWindow;
class folderTab;

class folderList : public QObject
{
    Q_OBJECT

public:
    folderList(QWidget *parent);
    MainWindow *mainwindow;

private:

    QDialog      *edit_folders_dialog,
    *dialog;

    QTabWidget     *tabholder;

    QWidget        *tab1,
    *tab2,
    *tab3,
    *tab4,
    *tab5,
    *tab6,
    *tab7;

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

    folderTab *harmonieTab,
    *brainlabTab;

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

class folderTab : public QWidget
{
    Q_OBJECT

public:
    folderTab(QWidget *parent = nullptr);
    MainWindow *mainwindow;
    void set_default_folder(QString folder);
    void set_recording_system(QString system);
    void set_folders(signal_dirs *s_d);
    void set_mainwindow(MainWindow *mainwindow_);
    signal_dirs *dirs_list;

private:
    QDialog *dialog;
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

    QString defaultDataFolder;
    QString recordingSystem;

    QListWidgetItem *currentItem;

    //    QLabel *staticLabel,
    //    *dynamicLabel,
    //    *helpLabel;

    //    QMessageBox *duplicate_msgBox;

    //    int row;
    bool currentMode;
    void rowClicked(QListWidgetItem *, bool dynamic);
    void add_folder(bool dynamic);
    void set_dynamic_folders(QStringList *dfolder_list);
    void set_static_folders(QStringList *sfolder_list);

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
