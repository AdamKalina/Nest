#include "mainwindow.h"


// ======== DATA LOADING ========

void MainWindow::loadDataFromDb(){

    if(db.isOpen()){
        // get IDs of X last patients ordered by date of last EEG
        QVector<QString> qpatientIds = db.getLastXPatientsId(nestOptions.patients2load_startup);

        QVectorIterator<QString> i(qpatientIds);
        while (i.hasNext()){
            QPatient qpatient = db.selectPatientbyIdWithRecords(i.next());
            QpatientQueue.enqueue(qpatient);
            IdMap.insert(qpatient.id,1);
        }
        dbLoaded = true;
        noOfPatientsLoaded = nestOptions.patients2load_startup;
    }
}

void MainWindow::loadMorePatients(){
    int newLoadedPatients = 0;
    if(db.isOpen()){
        // get IDs of X last patients ordered by date of last EEG
        QVector<QString> qpatientIds = db.getNextXPatientsId(noOfPatientsLoaded,nestOptions.patients2load_add);
        //qDebug() << qpatientIds;
        // now iterate over the IDs and load only those not already in the treeview
        QVectorIterator<QString> i(qpatientIds);
        while (i.hasNext()){
            QString qpatientID = i.next();
            QMap<QString, bool>::iterator qit = IdMap.find(qpatientID);
            if (qit != IdMap.end()) {
                // if the ID of QPatient is already loaded, do nothing
                qDebug() << "This patient is already loaded!";
                noOfPatientsLoaded += 1; // increase this too because otherwise the fetching would not work if some older patients were loaded via filterline
            }
            else{
                QPatient qpatient = db.selectPatientbyIdWithRecords(qpatientID);
                QpatientQueue.enqueue(qpatient); // add patient to buffer
                IdMap.insert(qpatient.id,1); // register patient to IDmap
                newLoadedPatients++;
                noOfPatientsLoaded += 1; // increment no of loaded patients
            }
        }
        qDebug() << "no_of_patients_loaded" << noOfPatientsLoaded << "- of that " << newLoadedPatients << " new.";
        if(newLoadedPatients != 0){ //when new patients were loaded
            updatePatientTreeModel();
            //treeView->viewport()->update(); //refresh the model - no need to call it here
        }
    }
}



QRecord MainWindow::getQRecord(QFileInfo fileInfo, const QString recordingSystem){
    //    qDebug() << "MainWindow::getQRecord";
    //    qDebug() << fileInfo.absoluteDir().absolutePath();
    //    qDebug() << recordingSystem;

    QRecord qrecord;

    if (recordingSystem == "Brainlab"){
        read_signal_file brainlabReader;
        qrecord = brainlabReader.get_qrecord_brainlab(fileInfo);
    }

    if(recordingSystem == "Harmonie"){
        qrecord = read_harmonie_file(fileInfo);
    }

    // get the info from the file
    if(recordingSystem == "Nicolet"){
        read_nicolet_file nicoletReader = read_nicolet_file();
        qrecord = nicoletReader.get_qrecord_nicolet(fileInfo);

        // now read the info from db
        if(nestOptions.readNicOneDb){
            if (nic_db.isOpen()){
                qDebug() << "Nicolet Database is open!";
                qDebug() << "qrecord.guidStudyID: " << qrecord.file_id;
                qrecord.nicolet_record_id_db = nic_db.getStrStudyNo(qrecord.file_id);
                qDebug() << "nic_db.getStrStudyNo(qrecord.guidStudyID): " << qrecord.nicolet_record_id_db;
                qrecord.set_comment();
            }else{
                qDebug() << "Nicolet database is not open!";
            }

        }

        qrecord.report_flag = int(db.reportExists(qrecord.file_id));


    }
    return qrecord;
}

// ======== Go through files and collect fileInfo ========
// this is separate function because there is no way to tell how long it will take so QProgressDialog can not be used
void MainWindow::checkDataOnHDD(QString path2load, bool dynamic, const QString recordingSystem){
    //    qDebug() << "MainWindow::checkDataOnHDD";
    //    qDebug() << path2load << " " << recordingSystem;

    QDateTime now = QDateTime::currentDateTime();

    // QDirIterator - goes through files recursively

    QString system_extension;

    system_extension = system_extensions[recordingSystem];

    QDirIterator QDit(path2load, QStringList() << system_extension.toLower() << system_extension, QDir::Files, QDirIterator::Subdirectories);
    while (QDit.hasNext()){

        QString Qpath = QDit.next();
        QFileInfo fi(Qpath);

        // if this is dynamic folder and periodicRefreshMode != 0 --> decide whether to put the file in queue
        if(dynamic && nestOptions.periodicRefreshMode != 0){
            if(fi.lastModified().daysTo(now) > nestOptions.periodicRefreshMode){
                //qDebug() << "This file is too old - skipping";
                continue;
            }
        }
        fiQueue.enqueue(fi);
    }
    readDataOnHDD(path2load, dynamic, recordingSystem);
}

// ======== Go through fileinfo and read file headers ========
void MainWindow::readDataOnHDD(QString path2load, bool dynamic, const QString recordingSystem){
    //    qDebug() << "MainWindow::readDataOnHDD";
    //    qDebug() << path2load << " " << recordingSystem;

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
        prepareQRecord(fid, dynamic, recordingSystem);

        // update progress dialog
        ii++;
        progress.setValue(ii);
    }
    qDebug() << "reading data from " << path2load << " finished";
}


QRecord MainWindow::prepareQRecord(QFileInfo fileInfo, bool dynamic, const QString recordingSystem){
    //qDebug() << "MainWindow::prepareQRecord";

    QRecord qrecord = getQRecord(fileInfo, recordingSystem);

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

        if(program_is_starting){
            //qDebug() << "program is starting - not adding to queue!";
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
    connect(recordingFileWatcher, SIGNAL(fileChanged(QString)), this, SLOT(recordedFileChanged(QString)));
}

void MainWindow::recordedFileChanged(const QString & path){

    QFileInfo fi(path);

    // some editors might emit two signals when changing file and the first one points to file with zero size
    // https://stackoverflow.com/questions/59754532/why-does-qfilesystemwatcher-emit-multiple-signals-and-qfileinfo-for-the-first-t
    if(fi.size() == 0){
        return;
    }

    QRecord qrecord = getQRecord(fi, "Brainlab");

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
    //    qDebug() << "addQRecord2model";
    //    qDebug() << "parent: " << parent.data();
    //    qDebug() << "Qrecord name: " <<Qrecord.name;
    //    qDebug() << "Qrecord id: " << Qrecord.id;
    //    qDebug() << "Qrecord comment: " << Qrecord.comment;
    //qDebug() << "Qrecord id: " << Qrecord.check_flag << " " << Qrecord.class_code << " " << Qrecord.doctor << " " << Qrecord.file_name << " " << Qrecord.file_path << " " << Qrecord.file_size << " " << Qrecord.id <<
    //            Qrecord.name << " " << Qrecord.record_duration_s << " " << Qrecord.protocol << " " << Qrecord.record_start << " " << Qrecord.recording_flag << " " << Qrecord.video_flag;
    // add int ncol for the old way of coloring red

    QTime n(0, 0, 0);
    n = n.addSecs(Qrecord.record_duration_s);

    if(newRecord){
        //qDebug() << "if a new record, add it to parent";
        model->insertRows(ind, 1, parent); // adds a child to the previous item
        //qDebug() << "inserted row successfull";
    }

    // TO DO - nice way how to display recording system in model
    model->setData(model->index(ind, 0, parent), Qrecord.file_name, Qt::DisplayRole);
    //qDebug() << "set data file name";
    model->setData(model->index(ind, 1, parent), Qrecord.comment, Qt::DisplayRole);
    //qDebug() << "set data qinfo";
    model->setData(model->index(ind, 2, parent), Qrecord.record_start,Qt::DisplayRole);
    //qDebug() << "set data record start";
    model->setData(model->index(ind, 3, parent), n.toString("hh:mm:ss"), Qt::DisplayRole);
    //qDebug() << "set data time";
    model->setData(model->index(ind, 4, parent), Qrecord.file_path, Qt::DisplayRole);
    //qDebug() << "set data file path";
    model->setData(model->index(ind, 5, parent), Qrecord.recording_flag, Qt::DisplayRole);
    //qDebug() << "set data recording flag";
    model->setData(model->index(ind, 6, parent), Qrecord.recording_system, Qt::DisplayRole);
    //qDebug() << "set data doctor";
    model->setData(model->index(ind, 7, parent), Qrecord.file_id, Qt::DisplayRole);
    //qDebug() << "set data file_id " << Qrecord.file_id;
    model->setData(model->index(ind, 8, parent), Qrecord.report_flag, Qt::DisplayRole);
    //qDebug() << "all data set";

    // if file has video - show DVicon
    if (Qrecord.video_flag){
        model->setData(model->index(ind,0, parent), QIcon(":/images/DV_icon.png"), Qt::DecorationRole);
        //qDebug() << "added DV icon";
    }
    else{ // if file has no video or the video was deleted - hide DVicon
        model->setData(model->index(ind,0, parent), NULL, Qt::DecorationRole);
    }

    // if file has report - show icon
    if (Qrecord.report_flag){
        model->setData(model->index(ind, 1, parent), QIcon(":/images/report_icon.png"), Qt::DecorationRole);
        //qDebug() << "added DV icon";
    }
    else{ // if file has no report - remove the icon
        model->setData(model->index(ind,1, parent), NULL, Qt::DecorationRole);
    }
}

void addQPatient2model(QAbstractItemModel *model, QPatient Qpatient){
    //qDebug() << "addQPatient2model";

    // define patient
    model->insertRow(0);

    model->setData(model->index(0, 0), Qpatient.id);
    model->setData(model->index(0, 1), Qpatient.name);
    model->setData(model->index(0, 2), Qpatient.last_record,Qt::DisplayRole);
    model->setData(model->index(0, 3), Qpatient.no, Qt::DisplayRole);
    model->setData(model->index(0, 3), Qt::AlignCenter, Qt::TextAlignmentRole);

    const QModelIndex parent = model->index(0,0); // get the item in the first row and first column

    // iterate through records
    int ind = 0;
    int ncol = 9;
    model->insertColumns(0, ncol, parent); // adds a child to the previous item

    QMap<QString, QRecord>::iterator to = Qpatient.Qrecords_map.begin();
    for (Qpatient.Qrecords_map.begin();to!=Qpatient.Qrecords_map.end(); ++to){
        addQRecord2model(model, ind, parent, to.value(),1); // newRecord = 1 --> append new record
        ind++;
    }
}

QAbstractItemModel* MainWindow::createPatientTreeModel(){

    QStandardItemModel *model = new QStandardItemModel(0, 8, this);

    model->setHeaderData(0, Qt::Horizontal, QString::fromLocal8Bit("Rodné èíslo"));
    model->setHeaderData(1, Qt::Horizontal, QString::fromLocal8Bit("Jméno"));
    model->setHeaderData(2, Qt::Horizontal, QString::fromLocal8Bit("Poslední EEG"));
    model->setHeaderData(3, Qt::Horizontal, QString::fromLocal8Bit("Poèet EEG"));
    model->setHeaderData(4, Qt::Horizontal, QString::fromLocal8Bit("Cesta"));
    model->setHeaderData(5, Qt::Horizontal, QString::fromLocal8Bit("Recorded"));
    model->setHeaderData(6, Qt::Horizontal, QString::fromLocal8Bit("System"));
    model->setHeaderData(7, Qt::Horizontal, QString::fromLocal8Bit("file_id"));
    model->setHeaderData(8, Qt::Horizontal, QString::fromLocal8Bit("report_flag"));

    while (!QpatientQueue.isEmpty()){
        // adds the patient to the model
        addQPatient2model(model, QpatientQueue.dequeue());
    }

    return model;
}

void MainWindow::buildTreeView(){

    //qDebug() << "creating source model";
    sourceModel = createPatientTreeModel(); // create sourceModel
    //sourceModel = new TreeModel(this);

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
    treeView->setColumnHidden(6,true); // hide "system"
    treeView->setColumnHidden(7,true); // hide "file_id"
    //treeView->setColumnHidden(8,true); // hide "report_flag"
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
    treeView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    //treeView->header()->setMinimumSectionSize(50);
    //treeView->header()->resizeSection(0 /*column index*/, 25 /*width*/);
    //treeView->setWordWrap(true);

    //treeView->resizeColumnToContents(1); //nefunguje
    //    treeView->setColumnWidth(0,250); //funguje, ale musí se nastavit ruènì
    //    treeView->setColumnWidth(1,250);
    //    treeView->setColumnWidth(2,250);
    //    treeView->setColumnWidth(3,250);
    //    treeView->setColumnWidth(4,1500);

    connect(treeView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(double_click_tree(QModelIndex)));
    connect(treeView, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(ShowContextMenu(const QPoint &)));
    connect(treeView->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(verticalScrollingTree(int)));

    layout->addWidget(treeView);
};

void MainWindow::updatePatientTreeModel(){
    qDebug() << "MainWindow::updatePatientTreeModel()";

    // add new Qpatients from stack
    while (!QpatientQueue.isEmpty()){
        // adds patients (that were not in the model already) from the stack to the model
        addQPatient2model(sourceModel, QpatientQueue.dequeue());
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

void MainWindow::fetchMorePatients(){ // delete this?
    qDebug() << "Got the signal to fetch more patients in time!" << QDateTime::currentDateTime();
    qDebug() << "main Window is visible: " << this->isVisible();
    if (this->isVisible()){ // otherwise it would call the first X patients
        qDebug() << "no_of_patients_loaded" << noOfPatientsLoaded;
        loadMorePatients();
        updatePatientTreeModel();
    }
}

// ======================== SLOTS ========================


void MainWindow::verticalScrollingTree(int value){
    qDebug() << "value " << value;
    qDebug() << "max " << treeView->verticalScrollBar()->maximum();
    if(value == treeView->verticalScrollBar()->maximum()){
        //fetchMorePatients();
        loadMorePatients();
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

    bool hasReport = false;

    QModelIndex index = treeView->indexAt(pos);
    QVariant path = index.sibling(index.row(),4).data();
    //qDebug() << path;

    hasReport = index.sibling(index.row(),8).data().toBool();

    // deprecated - extra column with the report_flag was added
    // QVariant comment = index.sibling(index.row(),1).data(Qt::DecorationRole);
    //    qDebug() << comment.typeName();
    //    if(comment.isValid()){
    //        if(!strcmp(comment.typeName(),"QIcon")){ // if icon is set in the comment column - record has report
    //            hasReport = true;
    //        }
    //    }
    //    qDebug() << "hasReport" << hasReport;

    // TO DO - vytvoøit možnost smazat celého pacienta
    // TO DO - lepší možnost rozeznávat parenta od children?

    if(!path.isValid()){ // !path.isValid() = is parent
        // nothing here since xVision was discontinued
    }
    else{
        if(nestOptions.recordDeleteAllow || nestOptions.exportAllow){
            QMenu contextMenu(tr("Context menu"), this);

            QAction showReportAction(tr("Show report"), this);
            //deleteAction.setData(path);
            connect(&showReportAction, SIGNAL(triggered()), this, SLOT(showReport()));
            contextMenu.addAction(&showReportAction);

            QAction clipboardAction(tr("Copy path to clipboard"), this);
            connect(&clipboardAction, SIGNAL(triggered()), this, SLOT(copyPathToClipboard()));
            contextMenu.addAction(&clipboardAction);

            QAction exportAction(tr("Export to EDF"), this);
            exportAction.setData(path);
            connect(&exportAction, SIGNAL(triggered()), this, SLOT(exportToEDF()));
            contextMenu.addAction(&exportAction);

            QAction deleteAction(tr("Delete this record from database"), this);
            //deleteAction.setData(path);
            connect(&deleteAction, SIGNAL(triggered()), this, SLOT(deleteRecord()));
            contextMenu.addAction(&deleteAction);

            // when I tried to add actions to menu on "if", it stopped working, so I build it and then I delete what is not needed

            if(!hasReport){
                contextMenu.removeAction(&showReportAction);
            }

            if(!nestOptions.recordDeleteAllow){
                contextMenu.removeAction(&deleteAction);
            }

            if(!nestOptions.exportAllow){
                contextMenu.removeAction(&exportAction);
            }

            contextMenu.exec(treeView->mapToGlobal(pos));
        }
    }
}

void MainWindow::double_click_record(QModelIndex index){
    // forking for "recorded" and "still recording" files only makes sense in BrainLab environment
    // on Win 10 it should be removed

    QString path = index.sibling(index.row(),4).data().toString();
    QString system = index.sibling(index.row(),6).data().toString();
    QVariant recording_flag = index.sibling(index.row(),5).data();

    bool fileExists = QFileInfo::exists(path);

    if(!fileExists){
        QMessageBox fileDoesNotExist;
        fileDoesNotExist.setIcon(QMessageBox::Warning);
        fileDoesNotExist.setText(tr("Path to the file does not exist.\nCheck if the file exists and if all the necessary drives are connected."));
        fileDoesNotExist.setStandardButtons(QMessageBox::Ok);
        fileDoesNotExist.exec();
        return;
    }

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

    if(system == "Brainlab"){
        if (recording_flag.toBool()){
            if(nestOptions.brainlabControl.isEmpty()){
                int ret = setProgram.exec();
                if (ret == QMessageBox::Yes){
                    chooseBrainLabControl();
                }
                else{
                    return;
                }
            }
            openBrainLabControl(path);
        }
        else{
            if(nestOptions.brainlabReader.isEmpty()){
                int ret = setProgram.exec();
                if (ret == QMessageBox::Yes){
                    chooseBrainLabReader();
                }
                else{
                    return;
                }
            }
            myProcess->setProgram(this->nestOptions.brainlabReader);
            myProcess->start();
        }
    }
    if(system == "Harmonie"){
        if(nestOptions.harmonieReader.isEmpty()){
            int ret = setProgram.exec();
            if (ret == QMessageBox::Yes){
                chooseHarmonieReader();
            }
            else{
                return;
            }
        }
        myProcess->setProgram(this->nestOptions.harmonieReader);
        myProcess->start();
    }

    if(system == "Nicolet"){
        if(nestOptions.nicoletReader.isEmpty()){
            int ret = setProgram.exec();
            if (ret == QMessageBox::Yes){
                chooseNicoletReader();
            }
            else{
                return;
            }
        }
        myProcess->setProgram(this->nestOptions.nicoletReader);
        myProcess->start();
    }
}

// deprecated
void MainWindow::openBrainLabControl(QString path){
    QFileInfo fi(path);
    //qDebug() << fi.fileName();

    // check if the file is in the folder loaded as drive S:
    if(!QFileInfo::exists(brainLabDrive + fi.fileName())){
        qDebug() << "the file is not on the path";
        qDebug() << fi.path();
        // if the right drive is not loaded then look for correct batch file
        int ind = nestOptions.Brainlab_dirs.dynamic_dirs.indexOf(fi.path());
        if (ind != -1){
            // if there is batch file mapped to it - use it
            QString batchFile = nestOptions.batchFiles.at(ind);
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
    myProcess->setProgram(this->nestOptions.brainlabControl);
    myProcess->start();
}

void MainWindow::exportToEDF(){
    //using this: https://stackoverflow.com/questions/28646914/qaction-with-custom-parameter
    // TO DO: is there a canon way?, maybe https://forum.qt.io/topic/101448/argument-in-connect/4
    // TO DO: in other slots I simply read the index under the mouse
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
            chooseExportProgram();
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

void MainWindow::showReport(){
    QModelIndex index = treeView->selectionModel()->currentIndex();
    QString file_id = index.sibling(index.row(),7).data().toString();
    qDebug() << file_id;
    QReport qreport = db.getReportByFileId(file_id);

    qreport.datum = index.sibling(index.row(),2).data().toDate().toString("dd.MM.yyyy"); //add the date of recording

    if (!reportViewerWindow){
        reportViewerWindow = new reportViewer(this); // Creates a dialog instance if it does not already exist
        reportViewerWindow->setWindowFlags(Qt::Window); // set it as window, otherwise it would be displayed as part of the mainwindow
        reportViewerWindow->move(200,200); // offset of the new window, otherwise it would open open centered relative to the main window
    }

    reportViewerWindow->setText(qreport);
    if (!reportViewerWindow->isVisible()) reportViewerWindow->show(); // Only shows the dialog if it is not already shown.
}

void MainWindow::copyPathToClipboard(){
    QModelIndex index = treeView->selectionModel()->currentIndex();
    QString file_name = index.sibling(index.row(),4).data().toString();
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(file_name, QClipboard::Clipboard);
};

void MainWindow::double_click_patient(QModelIndex index){

    QModelIndex parent;

    // when I reimplement this for the parent index, it stops working when it is double clicked
    // so this function reimplements double click only for all non-parent siblings
    if(index.column() != 0){
        parent = index.sibling(index.row(),0);
    }

    if(treeView->isExpanded(parent)){
        treeView->setExpanded(parent, false);
    }else{
        treeView->setExpanded(parent, true);
    }
}


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
        if(!nestOptions.Brainlab_dirs.dynamic_dirs.contains(new_dir)){
            nestOptions.Brainlab_dirs.dynamic_dirs << new_dir;
        }
    }else{
        if(!nestOptions.Brainlab_dirs.static_dirs.contains(new_dir)){
            nestOptions.Brainlab_dirs.static_dirs << new_dir;
        }
    }
    checkDataOnHDD(new_dir,dynamic,"Brainlab");
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
// this would probably do the trick, but I am not sure whether the translation would work anymore

void MainWindow::chooseExternalProgram(QString instruction, QString file_type, QString & option_exe){

    QString temp = QFileDialog::getOpenFileName(this, tr(instruction.toStdString().c_str()), nestOptions.defaultReaderFolder, tr(file_type.toStdString().c_str()));

    if(temp.isEmpty()){
        return;
    }else{
        option_exe = temp;
    }
}

void MainWindow::chooseBrainLabReader(){

    QString temp = QFileDialog::getOpenFileName(this, tr("Choose BrainLab EEG reader"), nestOptions.defaultReaderFolder, tr("BrainLab reader (*.exe)"));

    if(temp.isEmpty()){
        return;
    }else{
        nestOptions.brainlabReader = temp;
    }
};

void MainWindow::chooseBrainLabControl(){

    QString temp = QFileDialog::getOpenFileName(this, tr("Choose BrainLab control"), nestOptions.defaultReaderFolder, tr("BrainLab control (*.exe)"));

    if(temp.isEmpty()){
        return;
    }else{
        nestOptions.brainlabControl = temp;
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

void MainWindow::chooseHarmonieReader(){
    QString temp = QFileDialog::getOpenFileName(0, tr("Choose Harmonie Browser"), "", tr("Harmonie Browser(*.exe)"));

    if(temp.isEmpty()){
        return;
    }else{
        nestOptions.harmonieReader = temp;
    }
};

void MainWindow::chooseNicoletReader(){
    QString temp = QFileDialog::getOpenFileName(0, tr("Choose Nicolet Browser"), "", tr("Nicolet Browser(*.exe)"));

    if(temp.isEmpty()){
        return;
    }else{
        nestOptions.nicoletReader = temp;
    }
};

void MainWindow::refreshDynamic(){

    checkFolders(nestOptions.Brainlab_dirs.dynamic_dirs,true, "Brainlab");
    checkFolders(nestOptions.Harmonie_dirs.dynamic_dirs,true, "Harmonie");
    checkFolders(nestOptions.Nicolet_dirs.dynamic_dirs,true, "Nicolet");
    updateLastRefreshTime();
    updatePatientTreeModel();
}

void MainWindow::refreshStatic(){

    QMessageBox msgBox;
    msgBox.setText(tr("Refreshing static folders might take some time"));
    msgBox.setInformativeText(tr("Do you really want to do it now?"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    int ret = msgBox.exec();

    if(ret == QMessageBox::Cancel){
        return;
    }
    else{
        checkFolders(nestOptions.Brainlab_dirs.static_dirs, false, "Brainlab");
        checkFolders(nestOptions.Harmonie_dirs.dynamic_dirs,false, "Harmonie");
        checkFolders(nestOptions.Nicolet_dirs.dynamic_dirs,false, "Nicolet");
        updatePatientTreeModel();
    }
}

void MainWindow::checkFolders(const QStringList dirs, bool dynamic, const QString recordingSystem){
    // generic function - goes through list of folders and loads FileInfo into queue (inside checkDataOnHDD)
    if(!dirs.isEmpty()){
        for(const auto& i : dirs){
            qDebug() << "loading data: " << i;
            checkDataOnHDD(i, dynamic, recordingSystem);
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
    QString aboutQString = QString("EEGle Nest is originally a BrainLab record database using <a href='https://github.com/Frederik-D-Weber/sigtoedf'>convertSIGtoEDF</a> from Frederik-D-Weber to read BrainLab EEG files header.\n"
    "<br>"
    "Support for Stellate Harmonie and Natus NicoletOne files was added later.<br><br>"
    "Built using Qt Creator 4.14.1 and Qt %1 (%2)<br><br>"
    "Developed by Adam Kalina, Department of Neurology, Second Faculty of Medicine, Charles University and Motol University Hospital, 2021, during COVID-19").arg(QT_VERSION_STR, compiler);
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

    if (text != ""){
        filterLineHintTimer = new QTimer(this);
        filterLineHintTimer->setSingleShot(true);
        filterLineHintTimer->start(2000);
        connect(filterLineHintTimer, SIGNAL(timeout()), this, SLOT(showFilterLineHint()));
    }
}

void MainWindow::showFilterLineHint()
{
    //qDebug() << "MainWindow::showFilterLineHint()";
    QPoint point = QPoint(geometry().left() + filter->geometry().left(), geometry().top() + filter->geometry().bottom()+10);
    QToolTip::showText(point,tr("Looking for something? Try hitting ENTER!", nullptr, 1000));
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

    // ====== refresh dynamic action
    colapseAllAction = new QAction(tr("Collapse all"),this);
    colapseAllAction->setIcon(style()->standardIcon(QStyle::SP_TitleBarShadeButton));
    connect(colapseAllAction, SIGNAL(triggered()), this, SLOT(collapseAll()));

    // ======== File menu ========
    filemenu = new QMenu(this);
    filemenu->setTitle(tr("&Data"));
    //filemenu->addAction(tr("Add Static Folder"), this, SLOT(AddStaticFolderDialog()));
    //filemenu->addAction(tr("Add Dynamic Folder"), this, SLOT(AddDynamicFolderDialog()));
    filemenu->addAction(tr("Refresh Static"), this, SLOT(refreshStatic()));
    filemenu->addAction(refreshDynamicAction);
    filemenu->addAction(tr("Connect to storage"), this, SLOT(connect2storage()));

    // ======== Settings ========
    setmenu = new QMenu(this);
    setmenu->setTitle(tr("&Settings"));
    //setmenu->addAction(tr("Add Reader"), this, SLOT(chooseBrainLabReader()));
    //setmenu->addAction(tr("Add Control"), this, SLOT(chooseBrainLabControl()));
    setmenu->addAction(tr("Folders"), this, SLOT(editFolderList()));
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
    menubar->addAction(colapseAllAction);

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

    system_extensions["Brainlab"] = "*.SIG";
    system_extensions["Nicolet"] = "*.E";
    system_extensions["Harmonie"] = "*.STS";
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

void MainWindow::workingHoursQTimer(){ // fires every hour to check if it is working hours
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

void MainWindow::readNestSettings()
{
    nest_options nestSettingsReader;
    nestOptions = nestSettingsReader.readSettings();
}

void MainWindow::writeNestSettings()
{
    nest_options nestSettingsWriter;
    nestSettingsWriter.writeSettings(nestOptions);
}

// ======== CONNECT DB ========

void MainWindow::connectDb(){

    // Load db
    db.setPath(path2db);

    if (db.isOpen()){
        qDebug() << "Nest database is open!";
        db.createTablePatients();   // Creates a table if it doesn't exist. Otherwise, it will use existing table.
        db.createTableRecords();
        db.createTableReports();
        db.createIndexPatients();
    }else{
        qDebug() << "Nest database is not open!";
    }

    if(nestOptions.readNicOneDb){
        nic_db.connect(nestOptions.NicOneDbPath);
    }

}

MainWindow::~MainWindow()
{
    writeNestSettings(); //save setting in .ini file
    //qDebug() << "reportViewerWindow->close() " << reportViewerWindow->close();
}
