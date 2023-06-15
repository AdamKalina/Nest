#include "folderlist.h"

folderList::folderList(QWidget *w_parent)
{
    mainwindow = (MainWindow *)w_parent;

    QFont f( "Arial", 10, QFont::Bold);

    edit_folders_dialog = new QDialog;
    edit_folders_dialog->setMinimumSize(800, 265);
    edit_folders_dialog->setWindowTitle("Edit searched folders");
    edit_folders_dialog->setModal(true);
    edit_folders_dialog->setAttribute(Qt::WA_DeleteOnClose, true);

    tabholder = new QTabWidget;

    /////////////////////////////////////// tab 1 Brainlab folders ///////////////////////////////////////////////////////////////////////

    //    tab1 = new QWidget;

    //    QLabel *dynamicLabel = new QLabel(tr("Dynamic folders"));
    //    dynamicLabel->setFont(f);
    //    dfolder_path_list = new QListWidget;
    //    dfolder_path_list->setSelectionBehavior(QAbstractItemView::SelectRows);
    //    dfolder_path_list->setSelectionMode(QAbstractItemView::ExtendedSelection);
    //    dfolder_path_list->setSpacing(1);

    //    for(i=0; i < mainwindow->dynamic_dirs.size(); i++)
    //    {
    //        new QListWidgetItem(mainwindow->dynamic_dirs[i], dfolder_path_list);
    //    }

    //    add_dynamic_button = new QPushButton;
    //    add_dynamic_button->setText(tr("Add dynamic folder"));
    //    add_dynamic_button->setMinimumWidth(180);
    //    add_dynamic_button->setIcon(mainwindow->style()->standardIcon(QStyle::SP_FileDialogNewFolder));

    //    QLabel *staticLabel = new QLabel(tr("Static folders"));
    //    staticLabel->setFont(f);
    //    sfolder_path_list = new QListWidget;
    //    sfolder_path_list->setSelectionBehavior(QAbstractItemView::SelectRows);
    //    sfolder_path_list->setSelectionMode(QAbstractItemView::ExtendedSelection);
    //    sfolder_path_list->setSpacing(1);

    //    for(i=0; i < mainwindow->static_dirs.size(); i++)
    //    {
    //        new QListWidgetItem(mainwindow->static_dirs[i], sfolder_path_list);
    //    }

    //    add_static_button = new QPushButton;
    //    add_static_button->setText(tr("Add static folder"));
    //    add_static_button->setMinimumWidth(180);
    //    add_static_button->setIcon(mainwindow->style()->standardIcon(QStyle::SP_FileDialogNewFolder));

    //    refresh_sel_static_button = new QPushButton;
    //    refresh_sel_static_button->setText(tr("Refresh selected"));
    //    refresh_sel_static_button->setMinimumWidth(180);
    //    refresh_sel_static_button->setIcon(mainwindow->style()->standardIcon(QStyle::SP_BrowserReload));

    //    QLabel *helpLabel = new QLabel(tr("Double click on item for context menu"));
    //    helpLabel->setFont(f);

    //    //    QHBoxLayout *hlayout1 = new QHBoxLayout;
    //    //    hlayout1->addStretch(1000);
    //    //    hlayout1->addWidget(CloseButton);

    //    QHBoxLayout *hlayoutStatic = new QHBoxLayout;
    //    hlayoutStatic->addWidget(refresh_sel_static_button, Qt::AlignLeft);
    //    hlayoutStatic->addStretch(1000);
    //    hlayoutStatic->addWidget(add_static_button, Qt::AlignRight);

    //    QVBoxLayout *vlayout1 = new QVBoxLayout;
    //    vlayout1->addWidget(dynamicLabel);
    //    vlayout1->addWidget(dfolder_path_list, 1000);
    //    vlayout1->addWidget(add_dynamic_button, 1000, Qt::AlignRight);
    //    vlayout1->addWidget(staticLabel);
    //    vlayout1->addWidget(sfolder_path_list, 1000);
    //    vlayout1->addLayout(hlayoutStatic);
    //    vlayout1->addWidget(helpLabel);
    //    //    vlayout1->addSpacing(20);
    //    //    vlayout1->addLayout(hlayout1);

    //    tab1->setLayout(vlayout1);

    //edit_folders_dialog->setLayout(vlayout1);


    /////////////////////////////////////// tab 1 Brainlab folders ///////////////////////////////////////////////////////////////////////

    tab1 = new QWidget;

    brainlabTab = new folderTab;
    brainlabTab->set_recording_system("brainlab");
    brainlabTab->set_folders(&mainwindow->nestOptions.Brainlab_dirs);
    brainlabTab->set_mainwindow(mainwindow);

    QVBoxLayout *vlayout1 = new QVBoxLayout;

    vlayout1->addWidget(brainlabTab);
    vlayout1->setContentsMargins(0,0,0,0); // to cancel out nesting of layouts
    tab1->setLayout(vlayout1);

    /////////////////////////////////////// tab 2 Harmonie folders ///////////////////////////////////////////////////////////////////////

    tab2 = new QWidget;

    //folderTab *harmonieTab = new folderTab(this);
    //harmonieTab = new folderTab(this);
    harmonieTab = new folderTab;
    harmonieTab->set_recording_system("harmonie");
    harmonieTab->set_folders(&mainwindow->nestOptions.Harmonie_dirs);
    harmonieTab->set_mainwindow(mainwindow);

    QVBoxLayout *vlayout2 = new QVBoxLayout;

    vlayout2->addWidget(harmonieTab);
    vlayout2->setContentsMargins(0,0,0,0); // to cancel out nesting of layouts
    tab2->setLayout(vlayout2);

    /////////////////////////////////////// COMMONS ///////////////////////////////////////////////////////////////////////

    tabholder->addTab(tab1, "&BrainLab");
    tabholder->addTab(tab2, "&Harmonie");
    //tabholder->addTab(tab3, "&NicOne");

    CloseButton = new QPushButton;
    CloseButton->setText("Close");
    CloseButton->setMinimumWidth(180);
    CloseButton->setIcon(mainwindow->style()->standardIcon(QStyle::SP_DialogCloseButton));

    QHBoxLayout *horLayout = new QHBoxLayout;
    horLayout->addStretch(1000);
    horLayout->addWidget(CloseButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabholder);
    mainLayout->addSpacing(20);
    mainLayout->addLayout(horLayout);

    edit_folders_dialog->setLayout(mainLayout);

    // SIGNALS and SLOTS
    connect(CloseButton,   SIGNAL(clicked()), edit_folders_dialog, SLOT(close()));
    //    connect(dfolder_path_list, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(rowClickedDynamic(QListWidgetItem*)));
    //    connect(sfolder_path_list, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(rowClickedStatic(QListWidgetItem*)));
    //    connect(add_dynamic_button,SIGNAL(clicked()), this, SLOT(add_folder_dynamic()));
    //    connect(add_static_button,SIGNAL(clicked()), this, SLOT(add_folder_static()));
    //    connect(refresh_sel_static_button,SIGNAL(clicked()), this, SLOT(refresh_sel_static()));

    edit_folders_dialog->exec();
}

void folderList::rowClickedDynamic(QListWidgetItem *item){
    rowClicked(item,true);
}

void folderList::rowClickedStatic(QListWidgetItem *item){
    rowClicked(item,false);
}

void folderList::add_folder_dynamic(){
    add_folder(true);
}

void folderList::add_folder_static(){
    add_folder(false);
}

void folderList::refresh_sel_static(){

    for(int i = 0; i<sfolder_path_list->selectedItems().count(); i++){
        mainwindow->checkDataOnHDD(sfolder_path_list->selectedItems().at(i)->text(), false, "brainlab");
    }
    mainwindow->updatePatientTreeModel();
}

// TO DO simplify adding new folder, make some method for refreshing QWidgetItemList

void folderList::add_folder(bool dynamic){

    //prepare message box
    QMessageBox *duplicate_msgBox = new QMessageBox;
    duplicate_msgBox->setText(tr("Duplicate folder detected"));
    duplicate_msgBox->setInformativeText(tr("Returning without action"));

    QString new_dir = QFileDialog::getExistingDirectory(0, tr("Choose directory"), mainwindow->nestOptions.defaultDataFolder);
    if(new_dir.isEmpty()){
        return;
    }

    // add the folder only when it is not in the list already
    if (dynamic){
        if(!mainwindow->dynamic_dirs.contains(new_dir)){
            mainwindow->dynamic_dirs << new_dir;
            new QListWidgetItem(new_dir, dfolder_path_list);
        }
        else{
            duplicate_msgBox->exec();
        }
    }else{
        if(!mainwindow->static_dirs.contains(new_dir)){
            mainwindow->static_dirs << new_dir;
            new QListWidgetItem(new_dir, sfolder_path_list);
        }
        else{
            duplicate_msgBox->exec();
        }
    }
}


void folderList::rowClicked(QListWidgetItem *item, bool dynamic)
{
    currentItem = item;
    currentMode = dynamic;

    //qDebug() << item->data(Qt::DisplayRole);

    QVBoxLayout *vlayout_small = new QVBoxLayout;

    dialog = new QDialog(edit_folders_dialog);
    dialog->setMinimumSize(150, 270);
    dialog->setWindowTitle("Entry");
    dialog->setModal(true);
    dialog->setAttribute(Qt::WA_DeleteOnClose, true);

    button1 = new QPushButton(dialog);
    button1->setText("Edit");
    button1->setIcon(mainwindow->style()->standardIcon(QStyle::SP_DialogResetButton));

    button2 = new QPushButton(dialog);
    button2->setText("Remove");
    button2->setIcon(mainwindow->style()->standardIcon(QStyle::SP_TrashIcon));

    button3 = new QPushButton(dialog);
    button3->setText("Refresh");
    button3->setIcon(mainwindow->style()->standardIcon(QStyle::SP_BrowserReload));

    button4 = new QPushButton(dialog);
    button4->setText("Cancel");
    button4->setIcon(mainwindow->style()->standardIcon(QStyle::SP_DialogCancelButton));

    vlayout_small->addWidget(button1);
    vlayout_small->addWidget(button2);
    vlayout_small->addWidget(button3);
    vlayout_small->addWidget(button4);
    dialog->setLayout(vlayout_small);

    QObject::connect(button1, SIGNAL(clicked()), this,   SLOT(adEntry()));
    QObject::connect(button2, SIGNAL(clicked()), this,   SLOT(removeEntry()));
    QObject::connect(button3, SIGNAL(clicked()), this,   SLOT(refreshEntry()));
    QObject::connect(button4, SIGNAL(clicked()), dialog, SLOT(close()));

    dialog->exec();
}

void folderList::removeEntry(){

    if(currentMode){
        mainwindow->dynamic_dirs.removeOne(currentItem->data(Qt::DisplayRole).toString());
    }
    else{
        mainwindow->static_dirs.removeOne(currentItem->data(Qt::DisplayRole).toString());
    }
    delete currentItem;
    dialog->close();
}

void folderList::adEntry(){

    //prepare message box
    QMessageBox *duplicate_msgBox = new QMessageBox;
    duplicate_msgBox->setText(tr("Duplicate folder detected"));
    duplicate_msgBox->setInformativeText(tr("Returning without action"));

    QString new_dir = QFileDialog::getExistingDirectory(0, tr("Choose directory"), mainwindow->nestOptions.defaultDataFolder);
    if(new_dir.isEmpty()){
        return;
    }

    if(currentMode){
        // check if the value exists
        if(!mainwindow->dynamic_dirs.contains(new_dir)){
            //if not - replace existing value with new one
            int Cind = mainwindow->dynamic_dirs.indexOf(currentItem->data(Qt::DisplayRole).toString());
            mainwindow->dynamic_dirs.replace(Cind,new_dir);
        }
        else{
            duplicate_msgBox->exec();
            return;
        }
    }
    else{
        if(!mainwindow->static_dirs.contains(new_dir)){
            int Cind = mainwindow->static_dirs.indexOf(currentItem->data(Qt::DisplayRole).toString());
            mainwindow->static_dirs.replace(Cind,new_dir);
        }
        else{
            duplicate_msgBox->exec();
            return;
        }
    }
    currentItem->setData(Qt::DisplayRole,new_dir);
    dialog->close();
}

void folderList::refreshEntry(){
    mainwindow->checkDataOnHDD(currentItem->data(Qt::DisplayRole).toString(), currentMode, "brainlab");
    mainwindow->updatePatientTreeModel();
    dialog->close();
}


folderTab::folderTab(QWidget *parent)
{
    QFont f( "Arial", 10, QFont::Bold);
    QLabel *dynamicLabel = new QLabel(tr("Dynamic folders"));
    dynamicLabel->setFont(f);
    dfolder_path_list = new QListWidget;
    dfolder_path_list->setSelectionBehavior(QAbstractItemView::SelectRows);
    dfolder_path_list->setSelectionMode(QAbstractItemView::ExtendedSelection);
    dfolder_path_list->setSpacing(1);

    add_dynamic_button = new QPushButton;
    add_dynamic_button->setText(tr("Add dynamic folder"));
    add_dynamic_button->setMinimumWidth(180);
    add_dynamic_button->setIcon(this->style()->standardIcon(QStyle::SP_FileDialogNewFolder));

    QLabel *staticLabel = new QLabel(tr("Static folders"));
    staticLabel->setFont(f);
    sfolder_path_list = new QListWidget;
    sfolder_path_list->setSelectionBehavior(QAbstractItemView::SelectRows);
    sfolder_path_list->setSelectionMode(QAbstractItemView::ExtendedSelection);
    sfolder_path_list->setSpacing(1);

    add_static_button = new QPushButton;
    add_static_button->setText(tr("Add static folder"));
    add_static_button->setMinimumWidth(180);
    add_static_button->setIcon(this->style()->standardIcon(QStyle::SP_FileDialogNewFolder));

    refresh_sel_static_button = new QPushButton;
    refresh_sel_static_button->setText(tr("Refresh selected"));
    refresh_sel_static_button->setMinimumWidth(180);
    refresh_sel_static_button->setIcon(this->style()->standardIcon(QStyle::SP_BrowserReload));

    QLabel *helpLabel = new QLabel(tr("Double click on item for context menu"));
    helpLabel->setFont(f);

    QHBoxLayout *hlayoutStatic = new QHBoxLayout;
    hlayoutStatic->addWidget(refresh_sel_static_button, Qt::AlignLeft);
    hlayoutStatic->addStretch(1000);
    hlayoutStatic->addWidget(add_static_button, Qt::AlignRight);

    QVBoxLayout *vlayout1 = new QVBoxLayout(this);
    vlayout1->addWidget(dynamicLabel);
    vlayout1->addWidget(dfolder_path_list, 1000);
    vlayout1->addWidget(add_dynamic_button, 1000, Qt::AlignRight);
    vlayout1->addWidget(staticLabel);
    vlayout1->addWidget(sfolder_path_list, 1000);
    vlayout1->addLayout(hlayoutStatic);
    vlayout1->addWidget(helpLabel);

    connect(dfolder_path_list, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(rowClickedDynamic(QListWidgetItem*)));
    connect(sfolder_path_list, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(rowClickedStatic(QListWidgetItem*)));
    connect(add_dynamic_button,SIGNAL(clicked()), this, SLOT(add_folder_dynamic()));
    connect(add_static_button,SIGNAL(clicked()), this, SLOT(add_folder_static()));
    //connect(refresh_sel_static_button,SIGNAL(clicked()), this, SLOT(refresh_sel_static()));
}

void folderTab::add_folder_dynamic(){
    add_folder(true);
}

void folderTab::add_folder_static(){
    add_folder(false);
}

void folderTab::set_default_folder(QString folder){
    defaultDataFolder = folder;
}

void folderTab::set_recording_system(QString system){
    recordingSystem = system;
}

void folderTab::set_folders(signal_dirs *sd){
    dirs_list = sd;
    set_dynamic_folders(&dirs_list->dynamic_dirs);
    set_static_folders(&dirs_list->static_dirs);
}

void folderTab::set_dynamic_folders(QStringList *dfolder_list){
    int i;
    for(i=0; i < dfolder_list->size(); i++)
    {
        new QListWidgetItem(dfolder_list->at(i), dfolder_path_list);
    }
}

void folderTab::set_static_folders(QStringList *sfolder_list){
    int i;
    for(i=0; i < sfolder_list->size(); i++)
    {
        new QListWidgetItem(sfolder_list->at(i), sfolder_path_list);
    }
}

void folderTab::add_folder(bool dynamic){

    //qDebug() << recordingSystem;

    //prepare message box
    QMessageBox *duplicate_msgBox = new QMessageBox;
    duplicate_msgBox->setText(tr("Duplicate folder detected"));
    duplicate_msgBox->setInformativeText(tr("Returning withou action"));

    QString new_dir = QFileDialog::getExistingDirectory(0, tr("Choose directory"), defaultDataFolder);
    if(new_dir.isEmpty()){
        return;
    }

    // add the folder only when it is not in the list already
    if (dynamic){
        if(!dirs_list->dynamic_dirs.contains(new_dir)){
            dirs_list->dynamic_dirs << new_dir;
            new QListWidgetItem(new_dir, dfolder_path_list);
        }
        else{
            duplicate_msgBox->exec();
        }
    }else{
        if(!dirs_list->static_dirs.contains(new_dir)){
            dirs_list->static_dirs << new_dir;
            new QListWidgetItem(new_dir, sfolder_path_list);
        }
        else{
            duplicate_msgBox->exec();
        }
    }
}

void folderTab::set_mainwindow(MainWindow *mainwindow_){
    mainwindow = mainwindow_;
   //qDebug() << mainwindow->static_dirs;
    //qDebug() << dirs_list->static_dirs;
}

void folderTab::rowClickedDynamic(QListWidgetItem *item){
    rowClicked(item,true);
}

void folderTab::rowClickedStatic(QListWidgetItem *item){
    rowClicked(item,false);
}

void folderTab::rowClicked(QListWidgetItem *item, bool dynamic)
{
    currentItem = item;
    currentMode = dynamic;

    //qDebug() << item->data(Qt::DisplayRole);

    QVBoxLayout *vlayout_small = new QVBoxLayout;

    dialog = new QDialog(this);
    dialog->setMinimumSize(150, 270);
    dialog->setWindowTitle("Entry");
    dialog->setModal(true);
    dialog->setAttribute(Qt::WA_DeleteOnClose, true);

    button1 = new QPushButton(dialog);
    button1->setText("Edit");
    button1->setIcon(mainwindow->style()->standardIcon(QStyle::SP_DialogResetButton));

    button2 = new QPushButton(dialog);
    button2->setText("Remove");
    button2->setIcon(mainwindow->style()->standardIcon(QStyle::SP_TrashIcon));

    button3 = new QPushButton(dialog);
    button3->setText("Refresh");
    button3->setIcon(mainwindow->style()->standardIcon(QStyle::SP_BrowserReload));

    button4 = new QPushButton(dialog);
    button4->setText("Cancel");
    button4->setIcon(mainwindow->style()->standardIcon(QStyle::SP_DialogCancelButton));

    vlayout_small->addWidget(button1);
    vlayout_small->addWidget(button2);
    vlayout_small->addWidget(button3);
    vlayout_small->addWidget(button4);
    dialog->setLayout(vlayout_small);

    QObject::connect(button1, SIGNAL(clicked()), this,   SLOT(adEntry()));
    QObject::connect(button2, SIGNAL(clicked()), this,   SLOT(removeEntry()));
    QObject::connect(button3, SIGNAL(clicked()), this,   SLOT(refreshEntry()));
    QObject::connect(button4, SIGNAL(clicked()), dialog, SLOT(close()));

    dialog->exec();
}

void folderTab::removeEntry(){

    if(currentMode){
        dirs_list->dynamic_dirs.removeOne(currentItem->data(Qt::DisplayRole).toString());
    }
    else{
        dirs_list->static_dirs.removeOne(currentItem->data(Qt::DisplayRole).toString());
    }
    delete currentItem;
    dialog->close();
}

void folderTab::adEntry(){

    //prepare message box
    QMessageBox *duplicate_msgBox = new QMessageBox;
    duplicate_msgBox->setText(tr("Duplicate folder detected"));
    duplicate_msgBox->setInformativeText(tr("Returning without action"));

    QString new_dir = QFileDialog::getExistingDirectory(0, tr("Choose directory"), mainwindow->nestOptions.defaultDataFolder);
    if(new_dir.isEmpty()){
        return;
    }

    if(currentMode){
        // check if the value exists
        if(!dirs_list->dynamic_dirs.contains(new_dir)){
            //if not - replace existing value with new one
            int Cind = dirs_list->dynamic_dirs.indexOf(currentItem->data(Qt::DisplayRole).toString());
            dirs_list->dynamic_dirs.replace(Cind,new_dir);
        }
        else{
            duplicate_msgBox->exec();
            return;
        }
    }
    else{
        if(!dirs_list->static_dirs.contains(new_dir)){
            int Cind = dirs_list->static_dirs.indexOf(currentItem->data(Qt::DisplayRole).toString());
            dirs_list->static_dirs.replace(Cind,new_dir);
        }
        else{
            duplicate_msgBox->exec();
            return;
        }
    }
    currentItem->setData(Qt::DisplayRole,new_dir);
    dialog->close();
}

// pass EEG system as next argument
void folderTab::refreshEntry(){
    mainwindow->checkDataOnHDD(currentItem->data(Qt::DisplayRole).toString(), currentMode, recordingSystem);
    mainwindow->updatePatientTreeModel();
    dialog->close();
}
