#include "mainwindow.h"


// ======== LOAD DATA ========

void MainWindow::loadData(QString path2load){

    int ii = 0;

    // QDirIterator - goes through files recursively
    QDirIterator QDit(path2load, QStringList() << "*.sig" << "*.SIG", QDir::Files, QDirIterator::Subdirectories);
    while (QDit.hasNext()){

        QString Qpath = QDit.next();
        ii++;

        string path= Qpath.toLocal8Bit().data();
        //qDebug() << Qpath;

        // this function returns only the data needed - maybe rename it
        QRecord qrecord = read_signal_file(Qpath);

        // if something is wrong with this file, skip it
        if (qrecord.check_flag == 0){
            continue;
        }

        // check if .LOG file with same name exists - and if it does, flag the record as still being recorded
        // TO DO - is this the best way? There is lot of data in the header of recorded file, search for "TEMP" instead?
        // TO DO - the same for video file, is there a field in signal that states that video exists?

        QFileInfo fi(Qpath);

        qrecord.recording_flag = QFileInfo::exists(fi.canonicalPath() + "/" + fi.baseName() + ".LOG"); // bool to int
        qrecord.video_flag = QFileInfo::exists(fi.canonicalPath() + "/" + fi.baseName() + ".M01"); // bool to int

        // using QMap
        QMap<QString, QPatient>::iterator qit = patientMap.find(qrecord.id);
        if (qit != patientMap.end()) {
            // if QPatient already exists, add record to it
            // QMap = If there is already an item with the key key, that item's value is replaced with value
            // so it will rewrite data from dynamic folders
            // TO DO - check for duplicates here?
            qit->add_record(qrecord);
        }
        else{
            // if QPatient does not exist, create it
            QPatient Qpatient;
            Qpatient.set_values(qrecord);
            patientMap.insert(Qpatient.id, Qpatient);
        }
    }
    no_files_loaded = no_files_loaded + ii;
    qDebug() << "no of files processed: " << ii;
    //ii = 0;
}

void MainWindow::refreshData(QString path2load){

    QQueue<QFileInfo> fiQueue; // initiate queue for files

    // use QProgressDialog as busy indicator - minimum and maximum both are set to 0
    //QProgressDialog QDitProgress("Looking through files - wait for it", "Abort", 0, 0, this);
    //QDitProgress.setWindowModality(Qt::WindowModal);
    //QDitProgress.setCancelButton(nullptr);
    //QDitProgress.setMinimumDuration(0);
    QProgressBar QDitProgress;
    QDitProgress.setRange(0,0);

    QDateTime now = QDateTime::currentDateTime();

    // QDirIterator - goes through files recursively
    QDirIterator QDit(path2load, QStringList() << "*.sig" << "*.SIG", QDir::Files, QDirIterator::Subdirectories);
    while (QDit.hasNext()){

        QString Qpath = QDit.next();
        QFileInfo fi(Qpath);

        // put here what files shoud be enqeued
        if(periodicRefreshMode != 0){
            if(fi.lastModified().daysTo(now) > periodicRefreshMode)
                qDebug() << "This files is too old - skipping";
        }

        fiQueue.enqueue(fi);
    }

    //QDitProgress.close();
    QDitProgress.setValue(10);

    //qDebug() << "fiQueue size " << fiQueue.size();

    int ii = 0;

    QString ProgressLabel = QString("Refreshing data in folder %1").arg(path2load);
    QProgressDialog progress(ProgressLabel, "Abort", 0, fiQueue.size(), this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setCancelButton(nullptr);
    progress.setMinimumDuration(0);

    while (!fiQueue.isEmpty()){
        // this function returns only the data needed - maybe rename it

        QFileInfo fid = fiQueue.dequeue();

        QRecord qrecord = read_signal_file(fid.filePath());
        ii++;
        progress.setValue(ii);

        // if something is wrong with this file, skip it
        if (qrecord.check_flag == 0){
            continue;
        }

        // check if .LOG file with same name exists - and if it does, flag the record as still being recorded
        // TO DO - is this the best way? There is lot of data in the header of recorded file, search for "TEMP" instead?
        qrecord.recording_flag = QFileInfo::exists(fid.canonicalPath() + "/" + fid.baseName() + ".LOG"); // bool to int
        // TO DO - the same for video file, is there a field in signal that states that video exists?
        qrecord.video_flag = QFileInfo::exists(fid.canonicalPath() + "/" + fid.baseName() + ".M01"); // bool to int

        db.addRecord(qrecord);

        // using QMap
        QMap<QString, QPatient>::iterator qit = patientMap.find(qrecord.id);
        if (qit != patientMap.end()) {
            // if QPatient already exists, add record to it
            // QMap = If there is already an item with the key key, that item's value is replaced with value
            // so it will rewrite data from dynamic folders
            // TO DO - check for duplicates (with almost same ID) here?
            qit->add_record(qrecord);

            QMap<QString, QRecord>::iterator qot = qit->Qrecords_map.find(qrecord.file_name); // find the record in patient - conversion to QRecord is already done

            QrecordStack.push(qot.value()); // add record to buffer
        }
        else{
            // if QPatient does not exist, create it
            QPatient Qpatient;
            Qpatient.set_values(qrecord);
            patientMap.insert(Qpatient.id, Qpatient);
            QpatientStack.push(Qpatient); // add patient to buffer
        }
    }
}


void MainWindow::initLoadData(){
    if(static_dirs.isEmpty() && dynamic_dirs.isEmpty()){ // if there is no path to data it will ask for it right away
        AddFolderDialog("static");
    }
    else{
        if (!loadQMap()){
            // only when there is no QMap to load it goes through the static folders
            for (int j = 0; j < static_dirs.size(); ++j) {
                loadData(static_dirs.at(j));
                qDebug() << "here!";
            }
        }
        // now go through dynamic folders
        for (int j = 0; j < dynamic_dirs.size(); j++ ) {
            loadData(dynamic_dirs.at(j));
        }
        qDebug() << "total no of files processed: " << no_files_loaded;
        //buildTreeView();
    }
}

void MainWindow::updateLastCheckTime(){
    lastUpdateTime = lastUpdateTime.currentDateTime();
}

// ======== TREE MODEL ========
// TO DO - make this part of separate class


void addQRecord2model(QAbstractItemModel *model, int ind, QModelIndex parent, QRecord Qrecord, bool newRecord){
    // add int ncol for the old way of coloring red

    QTime n(0, 0, 0);

    if(newRecord){
        model->insertRows(ind, 1, parent); // adds a child to the previous item
    }

    model->setData(model->index(ind, 0, parent), Qrecord.file_name, Qt::DisplayRole);
    model->setData(model->index(ind, 1, parent), Qrecord.class_code, Qt::DisplayRole);

#if QT_VERSION >= 0x050800
    model->setData(model->index(ind, 2, parent), QDateTime::fromSecsSinceEpoch(Qrecord.record_start), Qt::DisplayRole);
#else
    model->setData(model->index(ind, 2, parent), QDateTime::fromTime_t(Qrecord.record_start), Qt::DisplayRole);
#endif
    model->setData(model->index(ind, 3, parent), n.addSecs(Qrecord.num_pages*10).toString("hh:mm:ss"), Qt::DisplayRole);
    model->setData(model->index(ind, 4, parent), Qrecord.file_path, Qt::DisplayRole);
    model->setData(model->index(ind, 5, parent), Qrecord.recording_flag, Qt::DisplayRole);
    model->setData(model->index(ind, 6, parent), Qrecord.doctor, Qt::DisplayRole);

    // if recording flag = 1 --> color the whole row red
//    if (Qrecord.recording_flag){
//        for (int i = 0;i < ncol;i++){
//            model->setData(model->index(ind, i, parent), QColor(Qt::red), Qt::ForegroundRole);
//        }
//    }

    QIcon *dvicon = new QIcon(":/images/DV_icon.png");

    // if file has video - show DVicon
    if (Qrecord.video_flag){
        model->setData(model->index(ind,0, parent), *dvicon, Qt::DecorationRole);
    }
}

void addQPatient2model(QAbstractItemModel *model, QString ID, QPatient Qpatient){

    // define patient
    model->insertRow(0);
    model->setData(model->index(0, 0), ID);
    model->setData(model->index(0, 1), Qpatient.name);

#if QT_VERSION >= 0x050800
    model->setData(model->index(0, 2), QDateTime::fromSecsSinceEpoch(Qpatient.last_record));
#else
    model->setData(model->index(0, 2), QDateTime::fromTime_t(Qpatient.last_record));
#endif


    model->setData(model->index(0, 3), Qpatient.no, Qt::DisplayRole);
    model->setData(model->index(0, 3), Qt::AlignCenter, Qt::TextAlignmentRole);

    const QModelIndex parent = model->index(0,0); // get the item in the first row and first column

    // iterate through records
    int ind = 0;
    int ncol = 7;
    model->insertColumns(0, ncol, parent); // adds a child to the previous item

    QTime n(0, 0, 0);

    QMap<QString, QRecord>::iterator to = Qpatient.Qrecords_map.begin();
    for (Qpatient.Qrecords_map.begin();to!=Qpatient.Qrecords_map.end(); ++to){

        addQRecord2model(model, ind, parent, to.value(),1); // newRecord = 1 --> append new record
        ind++;
    }
}


// make this part of separate class - TreeModel
QAbstractItemModel *createPatientTreeModel(QObject *parent, QMap<QString, QPatient> patientMap)
{
    QStandardItemModel *model = new QStandardItemModel(0, 7, parent);

    model->setHeaderData(0, Qt::Horizontal, QString::fromLocal8Bit("Rodné èíslo"));
    model->setHeaderData(1, Qt::Horizontal, QString::fromLocal8Bit("Jméno"));
    model->setHeaderData(2, Qt::Horizontal, QString::fromLocal8Bit("Poslední EEG"));
    model->setHeaderData(3, Qt::Horizontal, QString::fromLocal8Bit("Poèet EEG"));
    model->setHeaderData(4, Qt::Horizontal, QString::fromLocal8Bit("Cesta"));
    model->setHeaderData(5, Qt::Horizontal, QString::fromLocal8Bit("Recorded"));
    model->setHeaderData(6, Qt::Horizontal, QString::fromLocal8Bit("Doctor"));

    // iterate over patients using QMap
    QMap<QString, QPatient>::iterator qit = patientMap.begin();
    for (qit=patientMap.begin();qit!=patientMap.end();++qit){
        addQPatient2model(model,qit.key(), qit.value());
    }
    return model;
}

void MainWindow::buildTreeView(){

    treeView = new QTreeView;

    //qDebug() << "creating source model";
    sourceModel = createPatientTreeModel(this, patientMap); // create sourceModel
    sourceModelLoaded = 1;

    proxyModel = new LeafFilterProxyModel(this); // use this custom FilterProxyModel

    proxyModel->setSourceModel(sourceModel);

#if QT_VERSION >= 0x050A00
    proxyModel->setRecursiveFilteringEnabled(1); // This property was introduced in Qt 5.10.
#else
#endif
    proxyModel->setFilterKeyColumn(-1); // filter through all columns
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    treeView->setModel(proxyModel); // or set SourceModel here for no filtering
    treeView->setColumnHidden(4,1); // hide path to EEG file
    treeView->setColumnHidden(5,1); // hide "being recorded"
    treeView->setSortingEnabled(1); //enable sorting
    treeView->sortByColumn(2,Qt::DescendingOrder); //newest files first
    treeView->header()->setSectionsMovable(0); //disable moving columns by dragging
    treeView->header()->setDefaultAlignment(Qt::AlignCenter); //align header labels to center
    treeView->header()->setStretchLastSection(false); // do not stretch last section
    treeView->header()->setFont(QFont("Sans Serif", 12, QFont::Bold)); // set header font
    treeView->setEditTriggers(QAbstractItemView::NoEditTriggers); // turn off editing of existin data by user
    treeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    treeView->setAlternatingRowColors(1);
    treeView->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    treeView->setItemDelegate(new CustomDelegate); // set custom delegate
    treeView->expand(proxyModel->index(0,0)); // expands the patient with the newest record
    //treeView->expand(proxyModel->index(1,0)); // expands also the second patient
    connect(treeView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(double_click_tree(QModelIndex)));

    layout->addWidget(treeView);
};

void MainWindow::updatePatientTreeModel(){

    // hardcore delete all rows
    sourceModel->removeRows(0,sourceModel->rowCount());

    // iterate over patients again using QMap
    QMap<QString, QPatient>::iterator qit = patientMap.begin();
    for (qit=patientMap.begin();qit!=patientMap.end();++qit){
        addQPatient2model(sourceModel,qit.key(), qit.value());
    }

    treeView->expand(proxyModel->index(0,0)); // expands the patient with the newest record

}


void MainWindow::updatePatientTreeModel2(){

    while (!QpatientStack.isEmpty()){
        // adds the patient to the model
        QString patientID = QpatientStack.top().id;
        addQPatient2model(sourceModel, patientID, QpatientStack.pop());
    }

    while (!QrecordStack.isEmpty()){
        // finds patient with matching id and adds the record to it
        QString patientID = QrecordStack.top().id;
        QString recordID = QrecordStack.top().file_name;
        QModelIndexList parents = sourceModel->match(sourceModel->index(0,0), Qt::DisplayRole, patientID, 1, Qt::MatchExactly); // find matching row by patient ID, only one match is expected
        QModelIndex parentInd = parents.first();


        QModelIndexList childs = sourceModel->match(sourceModel->index(0,0,parentInd),Qt::DisplayRole, recordID,1,Qt::MatchExactly); // find matching row by file ID, only one match is expected

        if(childs.isEmpty()){
            incrementParentNo(parentInd);
            updateParentTime(parentInd);
            addQRecord2model(sourceModel, 0, parentInd, QrecordStack.pop(),1); // 1 = newRecord --> append new record
        }
        else{
            // TO DO - update existing records?
            int tempInd = childs.first().row();
            addQRecord2model(sourceModel, tempInd, parentInd, QrecordStack.pop(),0); // 0 = newRecord --> update existing
        }

    }
}

void MainWindow::incrementParentNo(QModelIndex parentInd){
    // increment parent number of EEG by 1
    QVariant QtempNo = sourceModel->data(parentInd.sibling(parentInd.row(),3));
    int tempNo = QtempNo.toInt();
    tempNo++;
    sourceModel->setData(parentInd.sibling(parentInd.row(),3),tempNo,Qt::DisplayRole);
}

void MainWindow::updateParentTime(QModelIndex parentInd){
    // change time of last EEG to current record - if it is newer
    QVariant QtempTime = sourceModel->data(parentInd.sibling(parentInd.row(),2));
    QDateTime QnewTime = TimeT2QDateTime(QrecordStack.top().record_start);
    if(QnewTime > QtempTime.toDateTime()){
        sourceModel->setData(parentInd.sibling(parentInd.row(),2),QnewTime,Qt::DisplayRole);
    }
}

QDateTime MainWindow::TimeT2QDateTime(time_t){
#if QT_VERSION >= 0x050800
    QDateTime QnewTime = QDateTime::fromSecsSinceEpoch(QrecordStack.top().record_start);
#else
    QDateTime QnewTime = QDateTime::fromTime_t(QrecordStack.top().record_start);
#endif

    return QnewTime;
}


void MainWindow::double_click_tree(QModelIndex index){
    //qDebug() << index.data(); // data pod kurzorem
#if QT_VERSION >= 0x050B00
    QVariant path = index.siblingAtColumn(4).data();
    QVariant recording_flag = index.siblingAtColumn(5).data();
#else
    QVariant path = index.sibling(index.row(),4).data();
    QVariant recording_flag = index.sibling(index.row(),5).data();
#endif

    //qDebug() << path;
    //qDebug() << recording_flag;

    // TO DO - check if the path exists?

    if (path.isValid()){ // in patient (where there is no path set) is not valid

        QMessageBox setProgram;
        setProgram.setIcon(QMessageBox::Question);
        setProgram.setText(tr("EEG reader is not set"));
        setProgram.setInformativeText(tr("Do you want to set it now?"));
        setProgram.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        setProgram.setDefaultButton(QMessageBox::Yes);
        //int ret = setProgram.exec();

        if (recording_flag.toBool()){
            if(externalProgram2.isEmpty()){
                int ret = setProgram.exec();
                if (ret == QMessageBox::Yes){
                    chooseExternalProgram2();
                }
                else{
                    return;
                }
            }

            //QMessageBox msgBox; // this messagebox is not really useful in XP - redirecting to control
            //msgBox.setIcon(QMessageBox::Warning);
            //msgBox.setText(tr("This is file is still being recorded, you will not able to review new data until you open it again."));
            //msgBox.exec();
            QProcess *myProcess = new QProcess(nullptr);
            QStringList arguments;
            arguments << path.toString();
            myProcess->setProgram(this->externalProgram2);
            myProcess->setArguments(arguments);
            myProcess->start();
        }
        else{
            if(externalProgram1.isEmpty()){
                int ret = setProgram.exec();
                if (ret == QMessageBox::Yes){
                    chooseExternalProgram1();
                }
                else{
                    return;
                }
            }
            QProcess *myProcess = new QProcess(nullptr);
            QStringList arguments;
            arguments << path.toString();
            myProcess->setProgram(this->externalProgram1);
            myProcess->setArguments(arguments);
            myProcess->start();
        }
    }
}

// ======== SLOTS ========

void MainWindow::AddStaticFolderDialog(){
    AddFolderDialog("static");
}

void MainWindow::AddDynamicFolderDialog(){
    AddFolderDialog("dynamic");
}

void MainWindow::AddFolderDialog(QString folder_type){

    new_dir = QFileDialog::getExistingDirectory(this, tr("Choose directory"), defaultDataFolder);
    if(new_dir.isEmpty()){
        return;
    }
    //qDebug() << new_dir;

    // add the folder only when it is not in the list already

    QStringList *dirs = nullptr;


    if (folder_type == "static"){
        dirs = &static_dirs;
    }else{
        dirs = &dynamic_dirs;
    }

    if (!dirs->contains(new_dir)){
        *dirs << new_dir;
    }
    loadData(new_dir);
    if(sourceModelLoaded){
        updatePatientTreeModel();
    }
    else{
        buildTreeView();
    }
};

void MainWindow::connect2storage(){
    // runs the batch command for storage connection
    QProcess connectProcess;
    connectProcess.setProgram("cmd.exe");
    QStringList arguments =  (QStringList() << "/C" << "connect.bat");
    connectProcess.setArguments(arguments);
    connectProcess.setWorkingDirectory(QDir::currentPath());
    connectProcess.start();
    connectProcess.waitForFinished(-1);
    qDebug() << connectProcess.readAllStandardOutput();
    qDebug() << connectProcess.readAllStandardError();
};


void MainWindow::chooseExternalProgram1(){

    QString temp = QFileDialog::getOpenFileName(this, tr("Choose EEG reader"), defaultReaderFolder, tr("BrainLab reader (*.exe)"));

    if(temp.isEmpty()){
        return;
    }
    else{
        externalProgram1 = temp;
    }
};

void MainWindow::chooseExternalProgram2(){

    QString temp = QFileDialog::getOpenFileName(this, tr("Choose EEG reader for control"), defaultReaderFolder, tr("BrainLab reader (*.exe)"));

    if(temp.isEmpty()){
        return;
    }
    else{
        externalProgram2 = temp;
    }
};

void MainWindow::refreshDynamic(){

    if(!dynamic_dirs.isEmpty()){
        for (int j = 0; j < dynamic_dirs.size(); j++ ) {
            qDebug() << "loading data: " << dynamic_dirs.at(j);
            refreshData(dynamic_dirs.at(j));
        }
    }
    updateLastCheckTime();
    updatePatientTreeModel2();
}

void MainWindow::refreshStatic(){

    QMessageBox msgBox;
    msgBox.setText(tr("Refreshing static folder might take some time"));
    msgBox.setInformativeText(tr("Do you really want to do it now?"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    int ret = msgBox.exec();

    if(ret == QMessageBox::Cancel){
        return;
    }
    else{
        if(!static_dirs.isEmpty()){
            for (int j = 0; j < static_dirs.size(); j++ ) {
                loadData(static_dirs.at(j));
            }
        }
        updatePatientTreeModel();
    }
}

void MainWindow::show_about_dialog()
{
    QString compiler = "undefined compiler";

    if (QT_VERSION == 0x50F02){
        compiler = "MinGW, 64-bit";
    }

    if(QT_VERSION == 0x050603){
        compiler = "MSVC 2013, 32-bit";
    }

    QMessageBox messagewindow;
    messagewindow.setIcon(QMessageBox::NoIcon);
    messagewindow.setText("About this program");
    QString aboutQString = QString("EEGle Nest is a BrainLab record database using convertSIGtoEDF from Frederik-D-Weber to read BrainLab EEG files header.\n"
    "\n"
    "Built using Qt Creator 4.14.1 and Qt %1 (%2)"
    "\n"
    "\n"
    "by Adam Kalina, Department of Neurology, Second Faculty of Medicine, Charles University and Motol University Hospital, 2021, during COVID-19").arg(QT_VERSION_STR, compiler);
    messagewindow.setInformativeText(aboutQString);
    messagewindow.setStyleSheet("QLabel{min-width: 700px;}");
    messagewindow.exec();
}

// ======== NO FILE WARNING ========

void MainWindow::showNoFileWarning(){
    QFont warning( "Arial", 10, QFont::Bold);
    QLabel *label = new QLabel(this);
    label->setFont(warning);
    label->setMargin(5);
    QString noFiles = QString::fromLocal8Bit("Žádné soubory k naètení, vyberte složku, ve které se nacházejí *.sig sobory pomocí 'Data --> Add Folder'");
    label->setText(noFiles);
    label->setAlignment(Qt::AlignCenter);
    setCentralWidget(label);
};

// ======== FILTER LINE ========

void MainWindow::buildFilterLine(){
    // text line for filtering
    filter = new QLineEdit(centralWidget);
    filter->setPlaceholderText(QString::fromLocal8Bit("jméno, rodné èíslo, datum yyyy-mm-dd"));
    filter->setClearButtonEnabled(1);
    connect(filter, SIGNAL(textChanged(QString)), this, SLOT(filter_text_changed(QString)));
    layout->addWidget(filter);
}

void MainWindow::filter_text_changed(const QString & text){
    //qDebug() << text;
    proxyModel->setFilterFixedString(text);
}

void MainWindow::notYetReady(){
    QMessageBox msgBox;
    msgBox.setText(tr("I am Sorry, this feature is not yet ready"));
    msgBox.setIcon(QMessageBox::Information);
    msgBox.exec();
}

void MainWindow::editFolderList()
{
    folderList(this);
}

void MainWindow::editProgramList()
{
    externalprogramlist(this);
}

void MainWindow::editRefreshSettings()
{
    refreshSettings(this);
}



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{

    // ======== Main Menu ========
    menubar = menuBar();

    // ======== File menu ========
    filemenu = new QMenu(this);
    filemenu->setTitle(tr("&Data"));
    filemenu->addAction(tr("Add Static Folder"), this, SLOT(AddStaticFolderDialog()));
    filemenu->addAction(tr("Add Dynamic Folder"), this, SLOT(AddDynamicFolderDialog()));
    filemenu->addAction(tr("Refresh Static"), this, SLOT(refreshStatic()));
    filemenu->addAction(tr("Refresh Dynamic"), this, SLOT(refreshDynamic())); // stack
    filemenu->addAction(tr("Connect to storage"), this, SLOT(connect2storage()));


    // ======== Settings ========
    setmenu = new QMenu(this);
    setmenu->setTitle(tr("&Settings"));
    setmenu->addAction(tr("Add Reader"), this, SLOT(chooseExternalProgram1()));
    setmenu->addAction(tr("Add Control"), this, SLOT(chooseExternalProgram2()));
    setmenu->addAction(tr("Folder list"), this, SLOT(editFolderList()));
    setmenu->addAction(tr("Program list"), this, SLOT(editProgramList()));
    setmenu->addAction(tr("Refresh settings"), this, SLOT(editRefreshSettings()));

    // ======== Help & About ========
    helpmenu = new QMenu(this);
    helpmenu->setTitle(tr("&Help"));
    helpmenu->addAction(tr("About"),this, SLOT(show_about_dialog()));
    helpmenu->addAction(tr("Manual"),this, SLOT(notYetReady()));


    menubar->addMenu(filemenu);
    menubar->addMenu(setmenu);
    menubar->addMenu(helpmenu);

    // ====== SHORTCUTS ======

    refreshKey = new QShortcut(QKeySequence::Refresh, this);
    connect(refreshKey,  SIGNAL(activated()), this, SLOT(refreshDynamic()));

    // ====== LAYOUT =====

    //set the layout
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    layout = new QVBoxLayout(centralWidget);

    // Filter line
    buildFilterLine(); // build filter line - do it first if you want it on the top


    // ====== APPLICATION CYCLE ======

    //    readSettings(); // read setting from .ini file
    //    buildFilterLine(); // build filter line - do it first if you want it on the top
    //    initLoadData(); //load data and update patientMap
    //    buildTreeView() // build tree view
    //    setUpQTimer();
    //    updateLastCheckTime();

    //    if (patientMap.size() == 0){
    //        showNoFileWarning(); // show warning that there are no files to load
    //    }

}


// ======== QTimers ========

void MainWindow::setUpRefreshQTimer(){
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(refreshDynamic()));
    refreshQTimer();
}

void MainWindow::refreshQTimer(){

    qDebug() << "refreshing period: " << refreshingPeriod << " minutes";
    qDebug() << "periodic Refreshing Enabled: " << periodicRefreshingEnabled;

    if (periodicRefreshingEnabled){
        timer->start(refreshingPeriod*60*1000); // in ms (from minutes)
    }else{
        timer->stop();
    }
}

void MainWindow::setUpWorkingHoursQTimer(){
    whTimer = new QTimer(this);
    connect(whTimer, SIGNAL(timeout()), this, SLOT(isItWorkingHours()));
    workingHoursQTimer();
}

void MainWindow::workingHoursQTimer(){
    if (workingHoursOnly){
        whTimer->start(1000*3600); // one hour
    }else{
        whTimer->stop();
    }
}

void MainWindow::isItWorkingHours(){
    int now = QTime::currentTime().hour();
    if(now > 7 && now < 17){
        periodicRefreshingEnabled = true;
    }else
    {
        periodicRefreshingEnabled = false;
    }
    refreshQTimer();
}


// ======== Write and Read Settings ========

void MainWindow::writeSettings()
{
    //QSettings settings("PuffinSoft", "EEGle Nest");
    QSettings settings("settings.ini",QSettings::IniFormat);
    settings.setValue("external_program_reader", externalProgram1);
    settings.setValue("external_program_control", externalProgram2);
    settings.setValue("path2QMap",QMapFile);
    settings.setValue("defaultDataFolder","D:/Dropbox/Scripts/Cpp/");
    settings.setValue("defaultReaderFolder","D:/Dropbox/Scripts/Cpp/EEGLE/build-EEGle-Desktop_Qt_5_15_2_MinGW_64_bit-Release/");
    settings.setValue("refreshing_period",refreshingPeriod);
    settings.setValue("periodic_refreshing_enabled", periodicRefreshingEnabled);
    settings.setValue("periodic_refreshing_in_working_hours_only",workingHoursOnly);
    settings.setValue("periodic_refresh_mode",periodicRefreshMode);

    settings.beginWriteArray("static_dirs");
    for (int i = 0; i < static_dirs.size(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue("path", static_dirs.at(i));
    }
    settings.endArray();

    settings.beginWriteArray("dynamic_dirs");
    for (int i = 0; i < dynamic_dirs.size(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue("path", dynamic_dirs.at(i));
    }
    settings.endArray();

}

void MainWindow::readSettings()
{
    QSettings settings("settings.ini",QSettings::IniFormat);
    externalProgram1 = settings.value("external_program_reader").toString();
    externalProgram2 = settings.value("external_program_control").toString();
    QMapFile = settings.value("path2QMap").toString();
    defaultDataFolder = settings.value("defaultDataFolder").toString();
    defaultReaderFolder = settings.value("defaultReaderFolder").toString();
    refreshingPeriod = settings.value("refreshing_period").toInt();
    periodicRefreshingEnabled = settings.value("periodic_refreshing_enabled").toBool();
    workingHoursOnly = settings.value("periodic_refreshing_in_working_hours_only").toBool();
    periodicRefreshMode = settings.value("periodic_refresh_mode").toInt();

    // load array of static folders
    int size = settings.beginReadArray("static_dirs");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QString new_stat_dir = settings.value("path").toString();
        static_dirs.append(new_stat_dir);
    }
    settings.endArray();


    // load array of dynamic folders
    size = settings.beginReadArray("dynamic_dirs");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QString new_dyn_dir = settings.value("path").toString();
        dynamic_dirs.append(new_dyn_dir);
    }
    settings.endArray();

    //qDebug() << "static directories: " << dynamic_dirs;
    //qDebug() << "dynamic directories: " << static_dirs;

}

// ======== connect db ========

void MainWindow::connectDb(){

    // Load db

    //DbManager db;

    db.setPath(path2db);

        if (db.isOpen())
        {
            db.createTablePatients();   // Creates a table if it doesn't exist. Otherwise, it will use existing table.
            db.createTableRecords();
            db.printAllPersons();
            qDebug() << "End";
        }
        else
        {
            qDebug() << "Database is not open!";
        }

}


// ======== WRITE and READ QMap ========

void MainWindow::saveQMap(){
    QMapFile = "patientMap.dat";
    QFile myFile(QMapFile);
    if (!myFile.open(QIODevice::WriteOnly))
    {
        qDebug() << "Could not write to file: " << QMapFile << "Error string:" << myFile.errorString();
        return;
    }

    QDataStream out(&myFile);
    out.setVersion(QDataStream::Qt_5_6);
    out << (quint32)0xD0AD; // = 53421 = digit sum = 6
    out << patientMap;
}


int MainWindow::loadQMap(){
    //QString filename = "patientMap.dat";
    QFile myFile(QMapFile);
    if (!myFile.exists()){
        qDebug() << "Could not read file: " << QMapFile << "Error string:" << myFile.errorString();
        return 0;
    }
    myFile.open(QIODevice::ReadOnly);
    QDataStream in(&myFile);

    in.setVersion(QDataStream::Qt_5_6);

    // Read and check the header
    quint32 magic;
    in >> magic;
    //qDebug() << magic;
    if (magic != 0xD0AD){
        qDebug() << "Wrong data format";
        return 0;
    }

    in >> patientMap;
    return 1;
}

MainWindow::~MainWindow()
{
    saveQMap();
    writeSettings(); //save setting in .ini file
}

