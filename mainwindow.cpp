#include "mainwindow.h"


// ======== DATA LOADING ========

void MainWindow::loadDataFromDb(){

    if(db.isOpen()){
        // get IDs of patients that had at least one recording in last x months

        // FOR TESTING - maybe use the last x months from before last record...


        if (nestOptions.months2load == 0){
            // check that always at least 2 months of data are loaded
            nestOptions.months2load = 2;
        }

        QVector<QString> qpatientIds = db.getPatientsIdsbyMonthsAgo(nestOptions.months2load);

        QVectorIterator<QString> i(qpatientIds);
        while (i.hasNext()){
            QPatient qpatient = db.selectPatientbyIdWithRecords(i.next());
            QpatientQueue.enqueue(qpatient);
            IdMap.insert(qpatient.id,1);
        }
        dbLoaded = true;
    }
}

QRecord MainWindow::getQRecord(QFileInfo fileInfo){

    // read the data header
    // this function returns only the data needed - maybe rename it
    QRecord qrecord = read_signal_file(fileInfo);

    // check if .LOG file with same name exists - and if it does, flag the record as still being recorded
    // TO DO - is this the best way? There is lot of data in the header of recorded file, search for "TEMP" instead?
    qrecord.recording_flag = QFileInfo::exists(fileInfo.canonicalPath() + "/" + fileInfo.baseName() + ".LOG"); // bool to int
    // TO DO - the same for video file, is there a field in signal that states that video exists?
    qrecord.video_flag = QFileInfo::exists(fileInfo.canonicalPath() + "/" + fileInfo.baseName() + ".M01"); // bool to int

    return qrecord;
}

// ======== Go through files and collect fileInfo ========
// this is separate function because there is no way to tell how long it will take so QProgressDialog can not be used
void MainWindow::checkDataOnHDD(QString path2load, bool dynamic){
    qDebug() << "MainWindow::checkDataOnHDD";

    QDateTime now = QDateTime::currentDateTime();

    // QDirIterator - goes through files recursively
    QDirIterator QDit(path2load, QStringList() << "*.sig" << "*.SIG", QDir::Files, QDirIterator::Subdirectories);
    while (QDit.hasNext()){

        QString Qpath = QDit.next();
        QFileInfo fi(Qpath);

        // if this is dynamic folder and periodicRefreshMode != 0 --> decide whether to put the file in queue
        if(dynamic && nestOptions.periodicRefreshMode != 0){
            if(fi.lastModified().daysTo(now) > nestOptions.periodicRefreshMode){
                qDebug() << "This file is too old - skipping";
                continue;
            }
        }
        fiQueue.enqueue(fi);
    }
    readDataOnHDD(path2load, dynamic);
}

// ======== Go through fileinfo and read file headers ========
void MainWindow::readDataOnHDD(QString path2load, bool dynamic){
    qDebug() << "MainWindow::readDataOnHDD";

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
        prepareQRecord(fid, dynamic);

        // update progress dialog
        ii++;
        progress.setValue(ii);
    }
    qDebug() << "reading data from " << path2load << " finished";
}


QRecord MainWindow::prepareQRecord(QFileInfo fileInfo, bool dynamic){
    qDebug() << "MainWindow::prepareQRecord";

    QRecord qrecord = getQRecord(fileInfo);

    // if something is wrong with this file, skip it
    if (qrecord.check_flag == 1){
        db.addRecord(qrecord); // add this record into database

        // if this record is still being recorded then add it to watcher
        if(qrecord.recording_flag == 1){
            recordingFileWatcher->addPath(fileInfo.filePath());
            qDebug() << "file "<< fileInfo.filePath() << "is being recorded = " << qrecord.recording_flag;
        }
        // skip loading files from static folders if that is enabled
        if(!dynamic && !nestOptions.refreshLoadStatic){
            qDebug() << "static folder - not adding to queue!";
            return qrecord;
        }

        if(programStart){
            qDebug() << "program is starting - not adding to queue!";
            return qrecord;
        }

        // using QMap - checks if the QPatient is already loaded
        QMap<QString, bool>::iterator qit = IdMap.find(qrecord.id);
        if (qit != IdMap.end()) {
            // if the ID of QPatient is already loaded, add the record to QrecordQueue (does not create new QPatient)
            QrecordQueue.enqueue(qrecord); // add record to queue
        }
        else{
            // if QPatient is not loaded, check if there are older records in db
            QPatient Qpatient = db.selectPatientbyIdWithRecords(qrecord.id);
            QpatientQueue.enqueue(Qpatient); // add patient to queue
            IdMap.insert(qrecord.id,1); // register patient to IDmap
        }
    }
    return qrecord;
}

// ====== WATCHERS ======

void MainWindow::initSystemWatcher(){

    // QFileSystemWatcher for files being recorded
    recordingFileWatcher = new QFileSystemWatcher(this);
    //recordingFileWatcher->addPath("D:/Dropbox/Scripts/Cpp/EEGLEnest/data_test/dynamic_data_test/S0023497.SIG");
    connect(recordingFileWatcher, SIGNAL(fileChanged(QString)), this, SLOT(recordedFileChanged(QString)));
}

void MainWindow::recordedFileChanged(const QString & path){

    QFileInfo fi(path);

    //some editors might emit two signals when changing file and the first one points to file with zero size
    // https://stackoverflow.com/questions/59754532/why-does-qfilesystemwatcher-emit-multiple-signals-and-qfileinfo-for-the-first-t
    if(fi.size() == 0){
        return;
    }

    QRecord qrecord = getQRecord(fi);

    if(qrecord.recording_flag == 1){
        //check it is still in the watcher
        if(!recordingFileWatcher->files().contains(path)){
            // if not, add it again
            // this is because recorder might delete old file and save it as new
            recordingFileWatcher->addPath(path);
        }
    }

    if(qrecord.recording_flag == 0){
        QString log = "file " + path + " has finished recording " + QDateTime::currentDateTime().toLocalTime().toString() + "\n";
        writeWatcherLog(log);

        // add it to queue for loading to treeView
        QrecordQueue.enqueue(qrecord);
        updatePatientTreeModel();
        // advice from here https://stackoverflow.com/questions/53589796/repaint-qtreewidget
        treeView->viewport()->update();
        // remove it from watcher
        recordingFileWatcher->removePath(path);
    }
}

void MainWindow::writeWatcherLog(QString log){
    // write to file that the file has finished recording
    QFile file("watcher.txt");
    if(file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
    {
        QTextStream stream(&file);
        //stream << "file " << path << " has finished recording " << QDateTime::currentDateTime().toLocalTime().toString() << "\n";
        stream << log;
        file.close();
    }
}

// ======== TREE MODEL ========
// TO DO - make this part of separate class?

void addQRecord2model(QAbstractItemModel *model, int ind, QModelIndex parent, QRecord Qrecord, bool newRecord){
    qDebug() << "addQRecord2model";
    qDebug() << "parent: " << parent.data();
    qDebug() << "Qrecord name: " <<Qrecord.name;
    qDebug() << "Qrecord id: " << Qrecord.id;
    qDebug() << "Qrecord id: " << Qrecord.check_flag << " " << Qrecord.class_code << " " << Qrecord.doctor << " " << Qrecord.file_name << " " << Qrecord.file_path << " " << Qrecord.file_size << " " << Qrecord.id <<
                Qrecord.name << " " << Qrecord.num_pages << " " << Qrecord.protocol << " " << Qrecord.record_start << " " << Qrecord.recording_flag << " " << Qrecord.video_flag;
    // add int ncol for the old way of coloring red
    QTime n(0, 0, 0);
    QTime t;
    t = n.addSecs(Qrecord.num_pages*10);

    qDebug() << "after time thingy";

    QString Qinfo;

    if(newRecord){
        qDebug() << "if a new record, add it to parent";
        model->insertRows(ind, 1, parent); // adds a child to the previous item
        qDebug() << "inserted row successfull";
    }


    if(Qrecord.doctor != "." && Qrecord.doctor != ""){
        Qinfo = Qrecord.class_code + "\n" + Qrecord.doctor;
    }else{
        Qinfo = Qrecord.class_code;
    }

    qDebug() << "after Qinfo thingy";

    model->setData(model->index(ind, 0, parent), Qrecord.file_name, Qt::DisplayRole);
    qDebug() << "set data file name";
    model->setData(model->index(ind, 1, parent), Qinfo, Qt::DisplayRole); //class_code
    qDebug() << "set data qinfo";
    model->setData(model->index(ind, 2, parent), Qrecord.record_start,Qt::DisplayRole);
    qDebug() << "set data record start";
    model->setData(model->index(ind, 3, parent), t.toString("hh:mm:ss"), Qt::DisplayRole);
    qDebug() << "set data time";
    model->setData(model->index(ind, 4, parent), Qrecord.file_path, Qt::DisplayRole);
    qDebug() << "set data file path";
    model->setData(model->index(ind, 5, parent), Qrecord.recording_flag, Qt::DisplayRole);
    qDebug() << "set data recording flag";
    model->setData(model->index(ind, 6, parent), Qrecord.doctor, Qt::DisplayRole);
    qDebug() << "set data doctor";
    qDebug() << "all data set";

    // if file has video - show DVicon
    if (Qrecord.video_flag){
        model->setData(model->index(ind,0, parent), QIcon(":/images/DV_icon.png"), Qt::DecorationRole);
        qDebug() << "added DV icon";
    }
}

void addQPatient2model(QAbstractItemModel *model, QPatient Qpatient, bool boldParent){
    qDebug() << "addQPatient2model";

    // define patient
    model->insertRow(0);

    model->setData(model->index(0, 0), Qpatient.id);
    model->setData(model->index(0, 1), Qpatient.name);
    model->setData(model->index(0, 2), Qpatient.last_record,Qt::DisplayRole);
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

    while (!QpatientQueue.isEmpty()){
        // adds the patient to the model
        addQPatient2model(model, QpatientQueue.dequeue(), nestOptions.boldParent);
    }

    return model;
}

void MainWindow::buildTreeView(){

    //qDebug() << "creating source model";
    sourceModel = createPatientTreeModel(); // create sourceModel

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
    treeView->setColumnHidden(6,true); // hide "doctor" - maybe delete it later
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
    treeView->setContextMenuPolicy(Qt::CustomContextMenu); // allow context menu

    connect(treeView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(double_click_tree(QModelIndex)));
    connect(treeView, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(ShowContextMenu(const QPoint &)));

    layout->addWidget(treeView);
};

void MainWindow::updatePatientTreeModel(){
    qDebug() << "MainWindow::updatePatientTreeModel()";

    // add new Qpatients from stack
    while (!QpatientQueue.isEmpty()){
        // adds patients (that were not in the model already) from the stack to the model
        addQPatient2model(sourceModel, QpatientQueue.dequeue(), nestOptions.boldParent);
    }

    while (!QrecordQueue.isEmpty()){
        // find patient with matching id in the model and add records from the stack to it

        QString patientID = QrecordQueue.head().id;
        QString recordID = QrecordQueue.head().file_name;

        QModelIndexList parents = sourceModel->match(sourceModel->index(0,0), Qt::DisplayRole, patientID, 1, Qt::MatchExactly); // find matching row by patient ID, only one match is expected
        // check if there is no match at all (e.g. when there was error in the patients ID)
        QModelIndex parentInd = parents.first();

        QModelIndexList childs = sourceModel->match(sourceModel->index(0,0,parentInd),Qt::DisplayRole, recordID, 1, Qt::MatchExactly); // find matching row by file ID, only one match is expected

        if(childs.isEmpty()){
            // record is not yet in the model, add it
            incrementParentNo(parentInd);
            updateParentTime(parentInd);
            addQRecord2model(sourceModel, 0, parentInd, QrecordQueue.dequeue(),1); // 1 = newRecord --> append new record
        }
        else{
            // record is already in the model, update it
            int tempInd = childs.first().row();
            addQRecord2model(sourceModel, tempInd, parentInd, QrecordQueue.dequeue(),0); // 0 = newRecord --> update existing
        }
    }
    treeView->expand(proxyModel->index(0,0)); // expands the patient with the newest record
}

void MainWindow::incrementParentNo(QModelIndex parentInd){
    qDebug() << "MainWindow::incrementParentNo";
    // increment parent number of EEG by 1
    QVariant QtempNo = sourceModel->data(parentInd.sibling(parentInd.row(),3));
    int tempNo = QtempNo.toInt();
    tempNo++;
    sourceModel->setData(parentInd.sibling(parentInd.row(),3),tempNo,Qt::DisplayRole);
}

void MainWindow::changeParentNoOfRecords(QModelIndex parentInd, int change){
    qDebug() << "MainWindow::changeParentNoOfRecords";
    // increase parent number of EEG by 1 or -1
    int tempNo = parentInd.sibling(parentInd.row(),3).data().toInt();
    tempNo += change;

    if(tempNo == 0){
        QString id = parentInd.data().toString();
        IdMap.remove(id); // remove patients from QMap
        proxyModel->removeRows(parentInd.row(),1,parentInd.parent()); // remove the patient from model
        db.removePerson(id); // remove the patients from database
    }
    else{
        proxyModel->setData(parentInd.sibling(parentInd.row(),3),tempNo,Qt::DisplayRole); //tady to funguje jen s proxyModelem - proè?
        // TO DO - update time of last EEG
        //qDebug() << "tady!" << change << tempNo;
        //qDebug() << "result = " << parentInd.sibling(parentInd.row(),3).data().toInt();
    }
}

void MainWindow::updateParentTime(QModelIndex parentInd){
    qDebug() << "MainWindow::updateParentTime";
    // change time of last EEG to current record - if it is newer
    QVariant QtempTime = sourceModel->data(parentInd.sibling(parentInd.row(),2));
    QDateTime QnewTime = QrecordQueue.head().record_start;
    if(QnewTime > QtempTime.toDateTime()){
        sourceModel->setData(parentInd.sibling(parentInd.row(),2),QnewTime,Qt::DisplayRole);
    }
}

void MainWindow::double_click_tree(QModelIndex index){
    //qDebug() << index.data(); // data pod kurzorem

    QVariant path = index.sibling(index.row(),4).data();

    if (path.isValid()){ // check if the path is valid (=not empty) == child with records
        double_click_record(index);
    }else{ // == parent
        double_click_patient(index);
    }
}

void MainWindow::ShowContextMenu(const QPoint & pos){

    QModelIndex index = treeView->indexAt(pos);
    QVariant path = index.sibling(index.row(),4).data();

    // TO DO - when the path.isValid() == false --> je to parent (pacient), vytvoøit možnost ho taky smazat

    if((nestOptions.recordDeleteAllow || nestOptions.exportAllow) && path.isValid()){
        qDebug() << path;
        QMenu contextMenu(tr("Context menu"), this);
        QAction exportAction(tr("Export to EDF"), this);
        exportAction.setData(path);
        connect(&exportAction, SIGNAL(triggered()), this, SLOT(exportToEDF()));
        contextMenu.addAction(&exportAction);

        QAction deleteAction(tr("Delete this record from database"), this);
        //deleteAction.setData(path);
        connect(&deleteAction, SIGNAL(triggered()), this, SLOT(deleteRecord()));
        contextMenu.addAction(&deleteAction);

        // when I tried to add actions to menu on "if", it stopped working, so I use this
        if(!nestOptions.recordDeleteAllow){
            contextMenu.removeAction(&deleteAction);
        }

        if(!nestOptions.exportAllow){
            contextMenu.removeAction(&exportAction);
        }

        contextMenu.exec(treeView->mapToGlobal(pos));
        //


    }
}

void MainWindow::double_click_record(QModelIndex index){
    // forking for recorded and still recording files only makes sense in BrainLab environment
    // on Win 10 it should be removed

    QString path = index.sibling(index.row(),4).data().toString();
    QVariant recording_flag = index.sibling(index.row(),5).data();

    // TO DO - check if the path exists?

    // TO DO - make a constructor for this messagebox somewhere else?
    QMessageBox setProgram;
    setProgram.setIcon(QMessageBox::Question);
    setProgram.setText(tr("EEG reader is not set"));
    setProgram.setInformativeText(tr("Do you want to set it now?"));
    setProgram.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    setProgram.setDefaultButton(QMessageBox::Yes);
    //int ret = setProgram.exec();

    QProcess *myProcess = new QProcess(nullptr);
    QStringList arguments;
    arguments << path;
    myProcess->setArguments(arguments);

    if (recording_flag.toBool()){
        if(nestOptions.externalProgram2.isEmpty()){
            int ret = setProgram.exec();
            if (ret == QMessageBox::Yes){
                chooseExternalProgram2();
            }
            else{
                return;
            }
        }
        openBrainLabControl(path);
    }
    else{
        if(nestOptions.externalProgram1.isEmpty()){
            int ret = setProgram.exec();
            if (ret == QMessageBox::Yes){
                chooseExternalProgram1();
            }
            else{
                return;
            }
        }
        myProcess->setProgram(this->nestOptions.externalProgram1);
        myProcess->start();
    }
}

void MainWindow::openBrainLabControl(QString path){
    QFileInfo fi(path);
    //qDebug() << fi.fileName();

    // check if the file is in the folder loaded as drive S:
    if(!QFileInfo::exists(brainLabDrive + fi.fileName())){
        qDebug() << "the file is not on the path";
        qDebug() << fi.path();
        // if the right drive is not loaded then look for correct batch file
        int ind = dynamic_dirs.indexOf(fi.path());
        if (ind != -1){
            // if there is batch file mapped to it - use it
            QString batchFile = batchFiles.at(ind);
            qDebug() << batchFile;
            runBatchFile(batchFile);
        }
    }else{
        qDebug() << "the file is on the path";
    }

    QProcess *myProcess = new QProcess(nullptr);
    QStringList arguments;
    arguments << path;
    myProcess->setArguments(arguments);
    myProcess->setProgram(this->nestOptions.externalProgram2);
    myProcess->start();
}

void MainWindow::exportToEDF(){
    //using this: https://stackoverflow.com/questions/28646914/qaction-with-custom-parameter
    // TO DO: is there a canon way?, maybe https://forum.qt.io/topic/101448/argument-in-connect/4
    QAction *act = qobject_cast<QAction *>(sender());
    QVariant v = act->data();
    qDebug() << v.toString();

    // check if exporter is set
    QMessageBox setProgram;
    setProgram.setIcon(QMessageBox::Question);
    setProgram.setText(tr("EEG exporter is not set"));
    setProgram.setInformativeText(tr("Do you want to set it now?"));
    setProgram.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    setProgram.setDefaultButton(QMessageBox::Yes);

    if(nestOptions.exportProgram.isEmpty()){
        int ret = setProgram.exec();
        if (ret == QMessageBox::Yes){
            chooseExternalProgram2(); //
        }
        else{
            return;
        }
    }else{
        QProcess *exportProcess = new QProcess(nullptr);
        QStringList arguments;
        arguments << v.toString();

        if(!nestOptions.exportPath.isEmpty()){
            QFileInfo fi(v.toString());
            QString str = nestOptions.exportPath + "/" + fi.baseName() + ".EDF";
            arguments << str;
        }

        if(nestOptions.exportAnonymize){
            arguments << "-a";
        }

        if(nestOptions.exportShortenLabels){
            arguments << "-s";
        }

        if(nestOptions.exportSystemEvents){
            arguments << "-y";
        }

        exportProcess->setArguments(arguments);
        exportProcess->setProgram(this->nestOptions.exportProgram);
        exportProcess->start();

        if(nestOptions.exportEnableDebug){
            exportProcess->waitForFinished(-1); //needed
            QString output(exportProcess->readAllStandardOutput());
            qDebug() << output;

            QString code(exportProcess->exitCode());
            qDebug() << code;
        }
    }
}

void MainWindow::deleteRecord(){
    QModelIndex index = treeView->selectionModel()->currentIndex();

    QString file_name = index.sibling(index.row(),0).data().toString();

    bool removeSuccess = proxyModel->removeRows(index.row(),1,index.parent()); // the magic was in calling the right model - did not work with sourceModel

    if (removeSuccess){
        //qDebug() << "success on removal " << removeSuccess;
        changeParentNoOfRecords(index.parent(), -1);
        db.removeRecord(file_name);
    }
}


void MainWindow::double_click_patient(QModelIndex index){

    QModelIndex parent;

    // when I reimplement this for the parent index, it stops working when it is double clicked
    // so this function reimplements double click only for all non-parent siblings
    if(index.column() != 0){
        parent = index.sibling(index.row(),0);
    }

    if(treeView->isExpanded(parent)){
        treeView->setExpanded(parent,false);
    }else{
        treeView->setExpanded(parent, true);
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

    new_dir = QFileDialog::getExistingDirectory(this, tr("Choose directory"), nestOptions.defaultDataFolder);
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
    checkDataOnHDD(new_dir,dynamic);
    updatePatientTreeModel();
};

void MainWindow::connect2storage(){
    runBatchFile("connect.bat");
};

void MainWindow::checkStorage(){

    QFile file("volumes.txt");
    if(file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
    {
        QTextStream stream(&file);
        stream << QDateTime::currentDateTime().toLocalTime().toString() << "\n";

        foreach (const QStorageInfo &storage, QStorageInfo::mountedVolumes()) {
            if (storage.isValid() && storage.isReady())
            {
                stream << "storage path " << storage.rootPath() << "\n";
                stream << "storage name " << storage.name() << "\n";
                stream << "storage readonly " << storage.isReadOnly() << "\n";
                stream << "storage filesystem type " << storage.fileSystemType() << "\n";
                stream << "storage device " << storage.device() << "\n\n";
            }
        }
        file.close();
    }
}

void MainWindow::runBatchFile(QString batchFile){
    // runs the batch command for storage connection
    QProcess connectProcess;
    connectProcess.setProgram("cmd.exe");
    QStringList arguments =  (QStringList() << "/C" << batchFile);
    connectProcess.setArguments(arguments);
    connectProcess.setWorkingDirectory(QDir::currentPath());
    connectProcess.start();
    connectProcess.waitForFinished();
    qDebug() << connectProcess.readAllStandardOutput();
    qDebug() << connectProcess.readAllStandardError();
}


// TO DO - generic function for adding external prorgram (BrainLab, BrainLab Control, BrainLab export, NicOne, Harmonie, etc.)

void MainWindow::chooseExternalProgram1(){

    QString temp = QFileDialog::getOpenFileName(this, tr("Choose EEG reader"), nestOptions.defaultReaderFolder, tr("BrainLab reader (*.exe)"));

    if(temp.isEmpty()){
        return;
    }else{
        nestOptions.externalProgram1 = temp;
    }
};

void MainWindow::chooseExternalProgram2(){

    QString temp = QFileDialog::getOpenFileName(this, tr("Choose EEG reader for control"), nestOptions.defaultReaderFolder, tr("BrainLab control (*.exe)"));

    if(temp.isEmpty()){
        return;
    }else{
        nestOptions.externalProgram2 = temp;
    }
};

void MainWindow::chooseExportProgram(){

    QString temp = QFileDialog::getOpenFileName(0, tr("Choose EDF exporter"), "", tr("Cuculus(*.exe)"));

    if(temp.isEmpty()){
        return;
    }else{
        nestOptions.exportProgram = temp;
    }
};

void MainWindow::refreshDynamic(){

    checkFolders(dynamic_dirs,true);
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
        checkFolders(static_dirs,false);
        updatePatientTreeModel();
    }
}

void MainWindow::checkFolders(const QStringList dirs, bool dynamic){
    // generic function - goes through list of folders and loads FileInfo into queue (inside checkDataOnHDD)
    if(!dirs.isEmpty()){
        for(const auto& i : dirs){
            qDebug() << "loading data: " << i;
            checkDataOnHDD(i,dynamic);
        }
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
    QString aboutQString = QString("EEGle Nest is a BrainLab record database using <a href='https://github.com/Frederik-D-Weber/sigtoedf'>convertSIGtoEDF</a> from Frederik-D-Weber to read BrainLab EEG files header.\n"
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
        treeView->setColumnHidden(4,false);
    }else{
        treeView->setColumnHidden(4,true);
    }
}

// ======== FILTER LINE ========

void MainWindow::buildFilterLine(){
    // text line for filtering
    filter = new QLineEdit();
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
    // TO DO - load only IDs at first and look for record only when the patient is not already loaded
    // if text = at least 6 numbers + X --> search ID

    QRegExp reId("\\d{6,}X{0,1}$");  // six or more digits (\d), with zero-to-one X on the end ($)
    reId.setCaseSensitivity(Qt::CaseInsensitive); // make it X or x on the end
    if (reId.exactMatch(filter->text())){
        qDebug() << filter->text() << " is proper ID";
        QPatient qpatient = db.selectPatientbyIdWithRecords(filter->text());
        checkQPatient(qpatient);
    }else{
        QRegExp reName("\\D*");
        if(reName.exactMatch(filter->text())){
            qDebug() << filter->text() << " is probably name";
            QPatient qpatient = db.selectPatientbyNameWithRecords(filter->text());
            if(qpatient.name.isEmpty()){
                fullTextSearch(filter->text());
            }else{
                checkQPatient(qpatient);
            }
        }else{
            fullTextSearch(filter->text());
        }
    }
}

// check if the patient entered via filterline is already loaded in treemodel
void MainWindow::checkQPatient(QPatient qpatient){
    QMap<QString, bool>::iterator qit = IdMap.find(qpatient.id);
    if (qit != IdMap.end()) {
        // if the ID of QPatient is already loaded, do nothing
        qDebug() << "This patient is already loaded!";
    }
    else{
        QpatientQueue.enqueue(qpatient); // add patient to buffer
        IdMap.insert(qpatient.id,1); // register patient to IDmap
    }
    updatePatientTreeModel();
}

// makes query from db using LIKE in selected columns
void MainWindow::fullTextSearch(QString query){
    QVector<QString> qpatientIds = db.getPatientsIdbyTextNote(query);
    qDebug() << qpatientIds;

    // now iterate over the IDs and load only those not already in the treeview
    QVectorIterator<QString> i(qpatientIds);
    while (i.hasNext()){
        QString qpatientID = i.next();
        QMap<QString, bool>::iterator qit = IdMap.find(qpatientID);
        if (qit != IdMap.end()) {
            // if the ID of QPatient is already loaded, do nothing
            qDebug() << "This patient is already loaded!";
        }
        else{
            QPatient qpatient = db.selectPatientbyIdWithRecords(qpatientID);
            QpatientQueue.enqueue(qpatient); // add patient to buffer
            IdMap.insert(qpatient.id,1); // register patient to IDmap
        }
    }
    updatePatientTreeModel();
    treeView->viewport()->update(); //refresh the model
    proxyModel->setFilterFixedString(query); //also refresh the view
}

// ======== MENU ========

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

void MainWindow::editTabOptions(){
    OptionsDialog(this);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{

    // ======== Main Menu ========
    menubar = menuBar();

    // ====== refresh dynamic action
    refreshDynamicAction = new QAction(tr("Refresh Dynamic"),this);
    refreshDynamicAction->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
    refreshDynamicAction->setToolTip("test");
    refreshDynamicAction->setWhatsThis("test");
    connect(refreshDynamicAction, SIGNAL(triggered()), this, SLOT(refreshDynamic()));

    // ======== File menu ========
    filemenu = new QMenu(this);
    filemenu->setTitle(tr("&Data"));
    filemenu->addAction(tr("Add Static Folder"), this, SLOT(AddStaticFolderDialog()));
    filemenu->addAction(tr("Add Dynamic Folder"), this, SLOT(AddDynamicFolderDialog()));
    filemenu->addAction(tr("Refresh Static"), this, SLOT(refreshStatic()));
    filemenu->addAction(refreshDynamicAction);
    filemenu->addAction(tr("Connect to storage"), this, SLOT(connect2storage()));

    // ======== Settings ========
    setmenu = new QMenu(this);
    setmenu->setTitle(tr("&Settings"));
    setmenu->addAction(tr("Add Reader"), this, SLOT(chooseExternalProgram1()));
    setmenu->addAction(tr("Add Control"), this, SLOT(chooseExternalProgram2()));
    setmenu->addAction(tr("Folders"), this, SLOT(editFolderList()));
    //setmenu->addAction(tr("Program list"), this, SLOT(editProgramList()));
    //setmenu->addAction(tr("Refresh settings"), this, SLOT(editRefreshSettings()));
    setmenu->addAction(tr("Options"), this, SLOT(editTabOptions()));

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
    menubar->addAction(refreshDynamicAction);

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

    // build parts of layout
    buildFilterLine(); // build filter line - do it first if you want it on the top
    buildTreeView();
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

    qDebug() << "refreshing period: " << nestOptions.refreshingPeriod << " minutes";
    qDebug() << "periodic Refreshing Enabled: " << nestOptions.periodicRefreshingEnabled;

    if (nestOptions.periodicRefreshingEnabled){
        timer->start(nestOptions.refreshingPeriod*60*1000); // in ms (from minutes)
    }else{
        timer->stop();
    }
}

// do not refresh automatically if it less then "refreshingPeriod" since last manual refresh
void MainWindow::isItTimeToRefresh(){
    if (lastRefreshTime.secsTo(QDateTime::currentDateTime()) > nestOptions.refreshingPeriod*60){
        refreshDynamic();
    }

}

void MainWindow::setUpWorkingHoursQTimer(){
    whTimer = new QTimer(this);
    connect(whTimer, SIGNAL(timeout()), this, SLOT(isItWorkingHours()));
    workingHoursQTimer();
}

void MainWindow::workingHoursQTimer(){ // fires every hour to check if it working hours
    if (nestOptions.refreshWorkingHoursOnly){
        whTimer->start(1000*3600); // one hour
    }else{
        whTimer->stop();
    }
}

void MainWindow::isItWorkingHours(){
    int now = QTime::currentTime().hour();
    if(now > 7 && now < 17){
        nestOptions.periodicRefreshingEnabled = true;
    }else
    {
        nestOptions.periodicRefreshingEnabled = false;
    }
    refreshQTimer();
}

// ======== Close event ========

void MainWindow::closeEvent (QCloseEvent *event)
{
    QMessageBox::StandardButton resBtn = QMessageBox::question( this, "EEGLE Nest",
                                                                tr("Are you sure you want to close EEGLE Nest?\nIt takes rather long time to start again.\n"),
                                                                QMessageBox::No | QMessageBox::Yes,
                                                                QMessageBox::No);
    if (resBtn != QMessageBox::Yes) {
        event->ignore();
    } else {
        event->accept();
    }
}




// ======== Write and Read Settings ========

void MainWindow::writeSettings()
{
    //QSettings settings("PuffinSoft", "EEGle Nest");
    QSettings settings("settings.ini",QSettings::IniFormat);
    settings.setValue("external_program_reader", nestOptions.externalProgram1);
    settings.setValue("external_program_control", nestOptions.externalProgram2);
    settings.setValue("defaultDataFolder","D:/Dropbox/Scripts/Cpp/");
    settings.setValue("defaultReaderFolder","D:/Dropbox/Scripts/Cpp/EEGLE/build-EEGle-Desktop_Qt_5_15_2_MinGW_64_bit-Release/");
    settings.setValue("refreshing_period",nestOptions.refreshingPeriod);
    settings.setValue("periodic_refreshing_enabled", nestOptions.periodicRefreshingEnabled);
    settings.setValue("load_static_on_refresh_enabled",nestOptions.refreshLoadStatic);
    settings.setValue("periodic_refreshing_in_working_hours_only",nestOptions.refreshWorkingHoursOnly);
    settings.setValue("periodic_refresh_mode",nestOptions.periodicRefreshMode);
    settings.setValue("bold_parent",nestOptions.boldParent);
    settings.setValue("anonymize_export",nestOptions.exportAnonymize);
    settings.setValue("export_program",nestOptions.exportProgram);
    settings.setValue("export_path",nestOptions.exportPath);
    settings.setValue("shorten_export",nestOptions.exportShortenLabels);
    settings.setValue("export_system_events",nestOptions.exportSystemEvents);
    settings.setValue("enable_delete_records",nestOptions.recordDeleteAllow);
    settings.setValue("enable_export_debug_mode",nestOptions.exportEnableDebug);
    settings.setValue("months_to_load",nestOptions.months2load);

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
    nestOptions.externalProgram1 = settings.value("external_program_reader").toString();
    nestOptions.externalProgram2 = settings.value("external_program_control").toString();
    nestOptions.defaultDataFolder = settings.value("defaultDataFolder").toString();
    nestOptions.defaultReaderFolder = settings.value("defaultReaderFolder").toString();
    nestOptions.refreshingPeriod = settings.value("refreshing_period").toInt();
    nestOptions.periodicRefreshingEnabled = settings.value("periodic_refreshing_enabled").toBool();
    nestOptions.refreshLoadStatic = settings.value("load_static_on_refresh_enabled").toBool();
    nestOptions.refreshWorkingHoursOnly = settings.value("periodic_refreshing_in_working_hours_only").toBool();
    nestOptions.periodicRefreshMode = settings.value("periodic_refresh_mode").toInt();
    nestOptions.boldParent = settings.value("bold_parent").toBool();
    nestOptions.exportAnonymize = settings.value("anonymize_export").toBool();
    nestOptions.exportProgram = settings.value("export_program").toString();
    nestOptions.exportPath = settings.value("export_path").toString();
    nestOptions.exportAllow = settings.value("allow_export").toBool();
    nestOptions.exportShortenLabels = settings.value("shorten_export").toBool();
    nestOptions.exportSystemEvents = settings.value("export_system_events").toBool();
    nestOptions.recordDeleteAllow = settings.value("enable_delete_records").toBool();
    nestOptions.exportEnableDebug = settings.value("enable_export_debug_mode").toBool();
    nestOptions.months2load = settings.value("months_to_load").toInt();

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

    // load array of used drives (not used)
    size = settings.beginReadArray("used_drives");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QString usedDrive = settings.value("path").toString();
        usedDrives.append(usedDrive);
    }
    settings.endArray();

    // load array of batch files - paired with used drives
    size = settings.beginReadArray("batch_files");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QString batchFile = settings.value("path").toString();
        batchFiles.append(batchFile);
    }
    settings.endArray();
}

// ======== CONNECT DB ========

void MainWindow::connectDb(){

    // Load db
    db.setPath(path2db);

    if (db.isOpen()){
        qDebug() << "Database is open!";
        db.createTablePatients();   // Creates a table if it doesn't exist. Otherwise, it will use existing table.
        db.createTableRecords();
        db.createIndexPatients();
    }else{
        qDebug() << "Database is not open!";
    }

}

MainWindow::~MainWindow()
{
    writeSettings(); //save setting in .ini file
}
