#include "mainwindow.h"


// ======== LOAD DATA ========

void MainWindow::loadDataFromDb(){

    // get IDs of patients that had at least one recording in last x months
    QVector<QString> qpatientIds = db.getPatientsIdsbyMonthsAgo(months2load);

    QVectorIterator<QString> i(qpatientIds);
    while (i.hasNext()){
        QPatient qpatient = db.selectPatientbyIdWithRecords(i.next());
        QpatientStack.push(qpatient);
        IdMap.insert(qpatient.id,1);
    }
    dbLoaded = true;
}

void MainWindow::loadDataFromHDD(QString path2load, bool dynamic){

    QQueue<QFileInfo> fiQueue; // initiate queue for filesInfo

    QDateTime now = QDateTime::currentDateTime();

    // ======== PART ONE - go through files and collect fileInfo ========
    // QDirIterator - goes through files recursively
    QDirIterator QDit(path2load, QStringList() << "*.sig" << "*.SIG", QDir::Files, QDirIterator::Subdirectories);
    while (QDit.hasNext()){

        QString Qpath = QDit.next();
        QFileInfo fi(Qpath);

        // if this is dynamic folder and periodicRefreshMode != 0 --> decide whether to put the file in queue
        if(dynamic && periodicRefreshMode != 0){
            if(fi.lastModified().daysTo(now) > periodicRefreshMode){
                //qDebug() << "This files is too old - skipping";
                continue;
            }
        }
        fiQueue.enqueue(fi);
    }

    // ======== PART TWO - go through file info and read file headers ========
    // TO DO - split this part into separate methods?


    //Progress dialog - shows the progress on reading files
    QString ProgressLabel = QString("Refreshing data in folder %1").arg(path2load);
    QProgressDialog progress(ProgressLabel, "Abort", 0, fiQueue.size(), this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setCancelButton(nullptr);
    progress.setMinimumDuration(0);
    int ii = 0;

    progress.setMaximum(fiQueue.size());

    while (!fiQueue.isEmpty()){

        QFileInfo fid = fiQueue.dequeue(); // take the next fileinfo from queue

        // read the data header
        // this function returns only the data needed - maybe rename it
        QRecord qrecord = read_signal_file(fid.filePath());

        // update progress dialog
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

        db.addRecord(qrecord); // add this record into databse


        // skip loading files from static folders if that is enabled
        if(!dynamic && !loadStaticOnRefreshEnabled){
            //qDebug() << "skipping!";
            continue;
        }

        // using QMap
        QMap<QString, bool>::iterator qit = IdMap.find(qrecord.id);
        if (qit != IdMap.end()) {
            // if the ID of QPatient is already loaded, add the record to QrecordStack (does not create new QPatient)
            QrecordStack.push(qrecord); // add record to buffer
        }
        else{
            // if QPatient does not exist, load it from db - check if there are older records
            QPatient Qpatient = db.selectPatientbyIdWithRecords(qrecord.id);
            QpatientStack.push(Qpatient); // add patient to buffer
            IdMap.insert(qrecord.id,1); // register patient to IDmap
        }
    }
}

void MainWindow::initLoadData(){
    if(static_dirs.isEmpty() && dynamic_dirs.isEmpty()){ // if there is no path to data it will ask for it right away
        AddFolderDialog(0);
    }
    else{
        if (!dbLoaded){
            // only when there is no databse data to load it goes through the static folders
            for (int j = 0; j < static_dirs.size(); ++j) {
                loadDataFromHDD(static_dirs.at(j),false);
            }
        }
        // now go through dynamic folders
        for (int j = 0; j < dynamic_dirs.size(); j++ ) {
            loadDataFromHDD(dynamic_dirs.at(j),true);
        }
        qDebug() << "total no of files processed: " << no_files_loaded;
    }
}

void MainWindow::initSystemWatcher(){
    // construct QFileSystemWatcher and add dynamic dirs to it
    watcher = new QFileSystemWatcher(dynamic_dirs, this);

    qDebug() << "Directories being watched " << watcher->directories();

    //connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(fileChanged(QString)));
    connect(watcher, SIGNAL(directoryChanged(QString)), this, SLOT(watchedDirChanged(QString)));


}

void MainWindow::watchedDirChanged(const QString & path){
    qDebug() << path << QDateTime::currentDateTime().toLocalTime();


    QFile file("watcher.txt");
          if(file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
          {
              // We're going to streaming text to the file
              QTextStream stream(&file);

              stream << path << " " << QDateTime::currentDateTime().toLocalTime().toString() << "\n";

              file.close();
              qDebug() << "Writing finished";
          }


}



// ======== TREE MODEL ========
// TO DO - make this part of separate class?

QDateTime TimeT2QDateTime(time_t t){ // no need to be part of MainWindow
#if QT_VERSION >= 0x050800
    QDateTime QnewTime = QDateTime::fromSecsSinceEpoch(t);
#else
    QDateTime QnewTime = QDateTime::fromTime_t(t);
#endif
    return QnewTime;
}

void addQRecord2model(QAbstractItemModel *model, int ind, QModelIndex parent, QRecord Qrecord, bool newRecord){
    // add int ncol for the old way of coloring red

    QTime nullTime(0, 0, 0);

    if(newRecord){
        model->insertRows(ind, 1, parent); // adds a child to the previous item
    }

    //QString test = Qrecord.class_code + "\\" + Qrecord.doctor;

    model->setData(model->index(ind, 0, parent), Qrecord.file_name, Qt::DisplayRole);
    model->setData(model->index(ind, 1, parent), Qrecord.class_code, Qt::DisplayRole);
    model->setData(model->index(ind, 2, parent), TimeT2QDateTime(Qrecord.record_start),Qt::DisplayRole);
    model->setData(model->index(ind, 3, parent), nullTime.addSecs(Qrecord.num_pages*10).toString("hh:mm:ss"), Qt::DisplayRole);
    model->setData(model->index(ind, 4, parent), Qrecord.file_path, Qt::DisplayRole);
    model->setData(model->index(ind, 5, parent), Qrecord.recording_flag, Qt::DisplayRole);
    model->setData(model->index(ind, 6, parent), Qrecord.doctor, Qt::DisplayRole);

    // if file has video - show DVicon
    if (Qrecord.video_flag){
        model->setData(model->index(ind,0, parent), QIcon(":/images/DV_icon.png"), Qt::DecorationRole);
    }
}

void addQPatient2model(QAbstractItemModel *model, QPatient Qpatient, bool boldParent){

    // define patient
    model->insertRow(0);

    model->setData(model->index(0, 0), Qpatient.id);
    model->setData(model->index(0, 1), Qpatient.name);
    model->setData(model->index(0, 2), TimeT2QDateTime(Qpatient.last_record),Qt::DisplayRole);
    model->setData(model->index(0, 3), Qpatient.no, Qt::DisplayRole);
    model->setData(model->index(0, 3), Qt::AlignCenter, Qt::TextAlignmentRole);

    //TO DO: do this in delegate
    if(boldParent){
        QFont boldFont;
        boldFont.setBold(true);
        for(int i = 0; i <= 4; i++ ){
            model->setData(model->index(0,i), boldFont,Qt::FontRole);
        }
    }

    const QModelIndex parent = model->index(0,0); // get the item in the first row and first column

    // iterate through records
    int ind = 0;
    int ncol = 7;
    model->insertColumns(0, ncol, parent); // adds a child to the previous item

    QMap<QString, QRecord>::iterator to = Qpatient.Qrecords_map.begin();
    for (Qpatient.Qrecords_map.begin();to!=Qpatient.Qrecords_map.end(); ++to){
        addQRecord2model(model, ind, parent, to.value(),1); // newRecord = 1 --> append new record
        ind++;
    }
}

QAbstractItemModel* MainWindow::createPatientTreeModel(){

    QStandardItemModel *model = new QStandardItemModel(0, 7, this);

    model->setHeaderData(0, Qt::Horizontal, QString::fromLocal8Bit("Rodné èíslo"));
    model->setHeaderData(1, Qt::Horizontal, QString::fromLocal8Bit("Jméno"));
    model->setHeaderData(2, Qt::Horizontal, QString::fromLocal8Bit("Poslední EEG"));
    model->setHeaderData(3, Qt::Horizontal, QString::fromLocal8Bit("Poèet EEG"));
    model->setHeaderData(4, Qt::Horizontal, QString::fromLocal8Bit("Cesta"));
    model->setHeaderData(5, Qt::Horizontal, QString::fromLocal8Bit("Recorded"));
    model->setHeaderData(6, Qt::Horizontal, QString::fromLocal8Bit("Doctor"));

    while (!QpatientStack.isEmpty()){
        // adds the patient to the model
        addQPatient2model(model, QpatientStack.pop(), boldParent);
    }

    return model;
}

void MainWindow::buildTreeView(){

    //qDebug() << "creating source model";
    sourceModel = createPatientTreeModel(); // create sourceModel
    sourceModelLoaded = 1;

    proxyModel = new LeafFilterProxyModel(this); // use this custom FilterProxyModel
    proxyModel->setSourceModel(sourceModel);

#if QT_VERSION >= 0x050A00
    proxyModel->setRecursiveFilteringEnabled(1); // This property was introduced in Qt 5.10.
#else
#endif
    proxyModel->setFilterKeyColumn(-1); // filter through all columns
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    treeView = new QTreeView;
    treeView->setModel(proxyModel); // or set SourceModel here for no filtering
    treeView->setColumnHidden(4,true); // hide path to EEG file
    treeView->setColumnHidden(5,true); // hide "being recorded"
    treeView->setSortingEnabled(true); // enable sorting
    treeView->sortByColumn(2,Qt::DescendingOrder); //newest files first
    treeView->header()->setSectionsMovable(0); // disable moving columns by dragging
    treeView->header()->setDefaultAlignment(Qt::AlignCenter); // align header labels to center
    treeView->header()->setStretchLastSection(false); // do not stretch last section
    treeView->header()->setFont(QFont("Sans Serif", 12, QFont::Bold)); // set header font
    treeView->setEditTriggers(QAbstractItemView::NoEditTriggers); // turn off editing of existing data by user
    treeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    treeView->setAlternatingRowColors(1);
    treeView->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    treeView->setItemDelegate(new CustomDelegate); // set custom delegate
    treeView->expand(proxyModel->index(0,0)); // expands the patient with the newest record

    connect(treeView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(double_click_tree(QModelIndex)));

    layout->addWidget(treeView);
};

void MainWindow::updatePatientTreeModel(){

    while (!QpatientStack.isEmpty()){
        // adds patients (that were not in the model already) from the stack to the model
        addQPatient2model(sourceModel, QpatientStack.pop(), boldParent);
    }

    while (!QrecordStack.isEmpty()){
        // find patient with matching id and add records from the stack to it
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

    if (path.isValid()){ // check if the path is valid (=not empty)

        QMessageBox setProgram;
        setProgram.setIcon(QMessageBox::Question);
        setProgram.setText(tr("EEG reader is not set"));
        setProgram.setInformativeText(tr("Do you want to set it now?"));
        setProgram.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        setProgram.setDefaultButton(QMessageBox::Yes);
        //int ret = setProgram.exec();

        QProcess *myProcess = new QProcess(nullptr);
        QStringList arguments;
        arguments << path.toString();
        myProcess->setArguments(arguments);


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
            myProcess->setProgram(this->externalProgram2);
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
            myProcess->setProgram(this->externalProgram1);
        }
        myProcess->start();
    }
}

// ======== SLOTS ========

void MainWindow::AddStaticFolderDialog(){
    AddFolderDialog(false);
}

void MainWindow::AddDynamicFolderDialog(){
    AddFolderDialog(true);
}

void MainWindow::AddFolderDialog(bool dynamic){

    new_dir = QFileDialog::getExistingDirectory(this, tr("Choose directory"), defaultDataFolder);
    if(new_dir.isEmpty()){
        return;
    }
    //qDebug() << new_dir;

    // add the folder only when it is not in the list already
    if (dynamic){
        if(!dynamic_dirs.contains(new_dir)){
            dynamic_dirs << new_dir;
        }
    }else{
        if(!static_dirs.contains(new_dir)){
            static_dirs << new_dir;
        }
    }
    loadDataFromHDD(new_dir,dynamic);
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

    QString temp = QFileDialog::getOpenFileName(this, tr("Choose EEG reader for control"), defaultReaderFolder, tr("BrainLab control (*.exe)"));

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
            loadDataFromHDD(dynamic_dirs.at(j),true);
        }
    }
    updateLastRefreshTime();
    updatePatientTreeModel();
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
                loadDataFromHDD(static_dirs.at(j),false);
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

// ======= View ======

void MainWindow::collapseAll(){
    treeView->collapseAll();
}

void MainWindow::expandAll(){
    treeView->expandAll();
}

void MainWindow::showPath(){
    if(showPathAction->isChecked()){
        qDebug() << "is checked!";
        treeView->setColumnHidden(4,false);
    }else{
        qDebug() << "is not checked";
        treeView->setColumnHidden(4,true);
    }

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
    connect(filter, SIGNAL(returnPressed()), this, SLOT(filter_return_pressed()));
    layout->addWidget(filter);
}

void MainWindow::filter_text_changed(const QString & text){
    //qDebug() << text;
    proxyModel->setFilterFixedString(text);
}

void MainWindow::filter_return_pressed(){
    // if text = at least 6 numbers + X --> search ID

    QRegExp reId("\\d{6,}X{0,1}$");  // six or more digits (\d), with zero-to-one X on the end ($)
    reId.setCaseSensitivity(Qt::CaseInsensitive); // make it X or X on the end
    if (reId.exactMatch(filter->text())){
        qDebug() << filter->text() << " is proper ID";
        QPatient qpatient = db.selectPatientbyIdWithRecords(filter->text());
        checkQPatient(qpatient);
    }else{
        QRegExp reName("\\D*");
        if(reName.exactMatch(filter->text())){
            qDebug() << filter->text() << " is probably name";
            QPatient qpatient = db.selectPatientbyNameWithRecords(filter->text());
            checkQPatient(qpatient);
        }
    }
}

void MainWindow::checkQPatient(QPatient qpatient){
    QMap<QString, bool>::iterator qit = IdMap.find(qpatient.id);
    if (qit != IdMap.end()) {
        // if the ID of QPatient is already loaded, do nothing
        qDebug() << "This patient is already loaded!";
    }
    else{
        QpatientStack.push(qpatient); // add patient to buffer
        IdMap.insert(qpatient.id,1); // register patient to IDmap
    }
    updatePatientTreeModel();
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

    // ======= View ========
    viewmenu = new QMenu(this);

    showPathAction = new QAction(tr("Show paths"), this);
    showPathAction->setCheckable(true);
    showPathAction->setChecked(false);

    viewmenu->setTitle(tr("&View"));
    viewmenu->addAction(tr("Collapse all"),this, SLOT(collapseAll()));
    viewmenu->addAction(tr("Expand all"),this, SLOT(expandAll()));
    viewmenu->addAction(showPathAction);

    connect(showPathAction,  SIGNAL(triggered()), this, SLOT(showPath()));

    // ======== Help & About ========
    helpmenu = new QMenu(this);
    helpmenu->setTitle(tr("&Help"));
    helpmenu->addAction(tr("About"),this, SLOT(show_about_dialog()));
    helpmenu->addAction(tr("Manual"),this, SLOT(notYetReady()));


    menubar->addMenu(filemenu);
    menubar->addMenu(setmenu);
    menubar->addMenu(viewmenu);
    menubar->addMenu(helpmenu);

    // ====== SHORTCUTS ======
    refreshKey = new QShortcut(QKeySequence::Refresh, this);
    connect(refreshKey,  SIGNAL(activated()), this, SLOT(refreshDynamic()));

    helpKey = new QShortcut(QKeySequence::HelpContents, this);
    connect(helpKey, SIGNAL(activated()), this, SLOT(show_about_dialog()));

    // ====== LAYOUT =====
    //set the layout
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    layout = new QVBoxLayout(centralWidget);

    // Filter line
    buildFilterLine(); // build filter line - do it first if you want it on the top
}


// ======== QTimers ========

void MainWindow::updateLastRefreshTime(){
    lastRefreshTime = lastRefreshTime.currentDateTime();
}

void MainWindow::setUpRefreshQTimer(){
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(isItTimeToRefresh()));
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

// do not refresh automatically if it less then "refreshingPeriod" since last manual refresh
void MainWindow::isItTimeToRefresh(){
    if (lastRefreshTime.secsTo(QDateTime::currentDateTime()) > refreshingPeriod*60){
        refreshDynamic();
    }

}

void MainWindow::setUpWorkingHoursQTimer(){
    whTimer = new QTimer(this);
    connect(whTimer, SIGNAL(timeout()), this, SLOT(isItWorkingHours()));
    workingHoursQTimer();
}

void MainWindow::workingHoursQTimer(){ // fires every hour to check if it working hours
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
    settings.setValue("load_static_on_refresh_enabled",loadStaticOnRefreshEnabled);
    settings.setValue("periodic_refreshing_in_working_hours_only",workingHoursOnly);
    settings.setValue("periodic_refresh_mode",periodicRefreshMode);
    settings.setValue("bold_parent",boldParent);

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
    loadStaticOnRefreshEnabled = settings.value("load_static_on_refresh_enabled").toBool();
    workingHoursOnly = settings.value("periodic_refreshing_in_working_hours_only").toBool();
    periodicRefreshMode = settings.value("periodic_refresh_mode").toInt();
    boldParent = settings.value("bold_parent").toBool();

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
}

// ======== CONNECT DB ========

void MainWindow::connectDb(){

    // Load db
    db.setPath(path2db);

    if (db.isOpen())
    {
        qDebug() << "Database is open!";
        db.createTablePatients();   // Creates a table if it doesn't exist. Otherwise, it will use existing table.
        db.createTableRecords();
        db.createIndexPatients();
    }
    else
    {
        qDebug() << "Database is not open!";
    }

}

MainWindow::~MainWindow()
{
    //saveQMap();
    writeSettings(); //save setting in .ini file
}
