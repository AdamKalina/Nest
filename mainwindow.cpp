#include "mainwindow.h"

string MainWindow::convert_time_for_sorting(const time_t * timer){ //deprecated
    // convert time
    char buffer [80];
    struct tm * timeinfo = localtime(timer);
    strftime (buffer,sizeof(buffer),"%Y-%m-%d %H:%M:%S",timeinfo);
    //qDebug() << buffer;
    return buffer;
}


// ======== LOAD DATA ========

void MainWindow::loadData(QString path2load){

    //qDebug() << path2load;
    // TO DO
    // store the mymap on HDD or use SQLite
    // add only new files - probably best secured by MyMap
    // report duplicates
    // dynamic folders
    // check if the files still exists?

    //    // test - go through the QDirIterator once and count files, then use it for qprogressdialog
    //    QElapsedTimer timer;
    //        timer.start();

    //    int iii = 0;
    //    QDirIterator testQDit(this->stat_dir, QStringList() << "*.sig" << "*.SIG", QDir::Files, QDirIterator::Subdirectories);
    //    while (testQDit.hasNext()){
    //        QString Qpath = testQDit.next();
    //    iii++;
    //    }
    //    qDebug() << "no of files which will be proccessed: " << iii;
    //    qDebug() << "The slow operation took" << timer.elapsed() << "milliseconds";

    //int count = 0;
    int ii = 0;

    // QDirIterator - goes through files recursively
    QDirIterator QDit(path2load, QStringList() << "*.sig" << "*.SIG", QDir::Files, QDirIterator::Subdirectories);
    while (QDit.hasNext()){

        QString Qpath = QDit.next();
        ii++;

        string path= Qpath.toLocal8Bit().data();
        //qDebug() << Qpath;

        // this function returns only the data needed - maybe rename it
        Record record = read_signal_file(path);

        // if something is wrong with this file, skip it
        if (record.check_flag == 0){
            continue;
        }

        // check if .LOG file with same name exists - and if it does, flag the record as still being recorded
        // TO DO - is this the best way? There is lot of data in the header of recorded file, search for "TEMP" instead?
        // TO DO - the same for video file, is there a field in signal that states that video exists?

        QFileInfo fi(Qpath);
        record.recording_flag = QFileInfo::exists(fi.canonicalPath() + "/" + fi.baseName() + ".LOG"); // bool to int
        record.video_flag = QFileInfo::exists(fi.canonicalPath() + "/" + fi.baseName() + ".M01"); // bool to int

        // using QMap
        QMap<QString, QPatient>::iterator qit = patientMap.find(QString::fromLocal8Bit(record.id.c_str()));
        if (qit != patientMap.end()) {
            // if QPatient already exists, add record to it
            // QMap = If there is already an item with the key key, that item's value is replaced with value
            // so it will rewrite data from dynamic folders
            // TO DO - check for duplicates here?
            qit->add_record(record);
        }
        else{
            // if QPatient does not exist, create it
            QPatient Qpatient;
            Qpatient.set_values(record);
            patientMap.insert(Qpatient.id, Qpatient);
        }

        // clasic map
        std::map<string, Patient>::iterator it;
        it = mymap.find(record.id);
        if (it != mymap.end()){ // if the patient already exists, add record to it
            //mymap.erase (it);
            //cout << it->first << " already exists" << endl;
            it->second.add_record(record);
        }
        else{ // if the patient does not exist, create him
            Patient patient;
            patient.set_values(record);
            // insert into map
            mymap.insert(std::pair<string,Patient>(patient.id,patient));
        }
    }
    no_files_loaded = no_files_loaded + ii;
    qDebug() << "no of files processed: " << ii;
    //ii = 0;
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
            }
        }
        qDebug() << "after static folders";
        // now go through dynamic folders
        for (int j = 0; j < dynamic_dirs.size(); j++ ) {
            loadData(dynamic_dirs.at(j));
        }
        qDebug() << "total no of files processed: " << no_files_loaded;
        buildTreeView();
    }

}

// ======== TREE MODEL ========

void addPatient2model(QAbstractItemModel *model, string ID, Patient patient){

    // define patient
    model->insertRow(0);

    model->setData(model->index(0, 0), QString::fromStdString(ID));
    model->setData(model->index(0, 1), QString::fromLocal8Bit(patient.name.c_str()));


#if QT_VERSION >= 0x050800
    model->setData(model->index(0, 2), QDateTime::fromSecsSinceEpoch(patient.last_record));
#else
    model->setData(model->index(0, 2), QDateTime::fromTime_t(patient.last_record));
#endif

    model->setData(model->index(0, 3), patient.no, Qt::DisplayRole);
    model->setData(model->index(0, 3), Qt::AlignCenter, Qt::TextAlignmentRole);
    //QDateTime last_record = QDateTime::fromSecsSinceEpoch(patient.last_record); // This function was introduced in Qt 5.8., before that use QDateTime::fromTime_t

    const QModelIndex parent = model->index(0,0); // get the item in the first row and first column

    // iterate through records
    int ind = 0;
    int ncol = 6;
    model->insertColumns(0, ncol, parent); // adds a child to the previous item

    QIcon *dvicon = new QIcon(":/images/DV_icon.png"); // or reuse it somehow? or use QPainter?

    std::map<string, Record>::iterator to = patient.records_map.begin();
    for (patient.records_map.begin();to!=patient.records_map.end(); ++to){
        model->insertRows(ind, 1, parent); // adds a child to the previous item
        model->setData(model->index(ind, 0, parent), QString::fromStdString(to->first), Qt::DisplayRole);
        model->setData(model->index(ind, 1, parent), QString::fromLocal8Bit(to->second.class_code.c_str()), Qt::DisplayRole);

#if QT_VERSION >= 0x050800
        model->setData(model->index(ind, 2, parent), QDateTime::fromSecsSinceEpoch(to->second.record_start), Qt::DisplayRole);
#else
        model->setData(model->index(ind, 2, parent), QDateTime::fromTime_t(to->second.record_start), Qt::DisplayRole);
#endif

        //model->setData(model->index(0, 3, parent), "", Qt::DisplayRole);
        model->setData(model->index(ind, 4, parent), QString::fromLocal8Bit(to->second.file_path.c_str()), Qt::DisplayRole);
        model->setData(model->index(ind, 5, parent), to->second.recording_flag, Qt::DisplayRole);

        // if recording flag = 1 --> color the whole row red
        // TO DO - use delegate?
        if (to->second.recording_flag){
            for (int i = 0;i < ncol;i++){
                model->setData(model->index(ind, i, parent), QColor(Qt::red), Qt::ForegroundRole);
            }
        }

        // if file has video - show DVicon
        if (to->second.video_flag){
            model->setData(model->index(ind,0, parent), *dvicon, Qt::DecorationRole);
        }

        ind++;
    }
    delete dvicon;
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
    //QDateTime last_record = QDateTime::fromSecsSinceEpoch(patient.last_record); // This function was introduced in Qt 5.8., before that use QDateTime::fromTime_t

    const QModelIndex parent = model->index(0,0); // get the item in the first row and first column

    // iterate through records
    int ind = 0;
    int ncol = 6;
    model->insertColumns(0, ncol, parent); // adds a child to the previous item

    QIcon *dvicon = new QIcon(":/images/DV_icon.png"); // or reuse it somehow? or use QPainter?

    QTime n(0, 0, 0);

    QMap<QString, QRecord>::iterator to = Qpatient.Qrecords_map.begin();
    for (Qpatient.Qrecords_map.begin();to!=Qpatient.Qrecords_map.end(); ++to){
        model->insertRows(ind, 1, parent); // adds a child to the previous item
        model->setData(model->index(ind, 0, parent), to.key(), Qt::DisplayRole);
        model->setData(model->index(ind, 1, parent), to.value().class_code, Qt::DisplayRole);

#if QT_VERSION >= 0x050800
        model->setData(model->index(ind, 2, parent), QDateTime::fromSecsSinceEpoch(to.value().record_start), Qt::DisplayRole);
#else
        model->setData(model->index(ind, 2, parent), QDateTime::fromTime_t(to.value().record_start), Qt::DisplayRole);
#endif

        model->setData(model->index(ind, 3, parent), n.addSecs(to.value().num_pages*10).toString("hh:mm:ss"), Qt::DisplayRole);
        model->setData(model->index(ind, 4, parent), to.value().file_path, Qt::DisplayRole);
        model->setData(model->index(ind, 5, parent), to.value().recording_flag, Qt::DisplayRole);

        // if recording flag = 1 --> color the whole row red
        // TO DO - use delegate?
        if (to.value().recording_flag){
            for (int i = 0;i < ncol;i++){
                model->setData(model->index(ind, i, parent), QColor(Qt::red), Qt::ForegroundRole);
            }
        }

        // if file has video - show DVicon
        if (to.value().video_flag){
            model->setData(model->index(ind,0, parent), *dvicon, Qt::DecorationRole);
        }

        ind++;
    }
}


// make this part of MainWindow
QAbstractItemModel *createPatientTreeModel(QObject *parent, std::map<string, Patient> mymap, QMap<QString, QPatient> patientMap)
{
    QStandardItemModel *model = new QStandardItemModel(0, 6, parent);

    model->setHeaderData(0, Qt::Horizontal, QString::fromLocal8Bit("Rodné èíslo"));
    model->setHeaderData(1, Qt::Horizontal, QString::fromLocal8Bit("Jméno"));
    model->setHeaderData(2, Qt::Horizontal, QString::fromLocal8Bit("Poslední EEG"));
    model->setHeaderData(3, Qt::Horizontal, QString::fromLocal8Bit("Poèet EEG"));
    model->setHeaderData(4, Qt::Horizontal, QString::fromLocal8Bit("Cesta"));
    model->setHeaderData(5, Qt::Horizontal, QString::fromLocal8Bit("Recorded"));
    // https://forum.qt.io/topic/123358/qtreeview-header-text-alignment/2

    //QIcon *DVicon = new QIcon(":/images/DV_icon.png");

    // iterate over patients - using std::map
    //std::map<string, Patient>::iterator it = mymap.begin();
    //for (it=mymap.begin(); it!=mymap.end(); ++it){
    //qDebug() << QString::fromLocal8Bit(it->first.c_str());
    //addPatient2model(model,it->first, it->second);
    //}

    // iterate over patients using QMap
    QMap<QString, QPatient>::iterator qit = patientMap.begin();
    for (qit=patientMap.begin();qit!=patientMap.end();++qit){
        addQPatient2model(model,qit.key(), qit.value());
    }

    return model;
}

void MainWindow::buildTreeView(){

    //delete treeView; // check whether this needs to be here

    treeView = new QTreeView;

    qDebug() << "creating source model";
    sourceModel = createPatientTreeModel(this, mymap, patientMap); // create sourceModel
    sourceModelLoaded = 1;

    //proxyModel = new QSortFilterProxyModel(this);
    proxyModel = new LeafFilterProxyModel(this); // use this custom FilterProxyModel

    proxyModel->setSourceModel(sourceModel);

#if QT_VERSION >= 0x051000
    proxyModel->setRecursiveFilteringEnabled(1); // This property was introduced in Qt 5.10. - Luckily LeafFilterProxyModel probably takes care of that
#else
#endif
    proxyModel->setFilterKeyColumn(-1); // filter through all columns
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    treeView->setModel(proxyModel); // or set SourceModel here for no filtering
    treeView->setColumnHidden(4,1); // hide path to EEG file
    treeView->setColumnHidden(5,1); // hide being recorded
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
    treeView->expand(proxyModel->index(0,0)); // expands the patient with the newest record
    //treeView->expand(proxyModel->index(1,0)); // expands the second patient
    connect(treeView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(double_click_tree(QModelIndex)));

    layout->addWidget(treeView);
};

void MainWindow::updatePatientTreeModel(){

    // to use it this properly I would need to subclass QAbstractItemModel and define reset
    //model->beginResetModel();

    // hardcore delete all rows
    sourceModel->removeRows(0,sourceModel->rowCount());

    // and add all patients and records again
    std::map<string, Patient>::iterator it = mymap.begin();
    for (it=mymap.begin(); it!=mymap.end(); ++it){
        //qDebug() << QString::fromLocal8Bit(it->first.c_str());
        //addPatient2model(sourceModel,it->first, it->second);
    }

    // iterate over patients again using QMap
    QMap<QString, QPatient>::iterator qit = patientMap.begin();
    for (qit=patientMap.begin();qit!=patientMap.end();++qit){
        addQPatient2model(sourceModel,qit.key(), qit.value());
    }

    treeView->expand(proxyModel->index(0,0)); // expands the patient with the newest record

}

void MainWindow::double_click_tree(QModelIndex index){
    //qDebug() << index.data(); // data pod kurzorem
#if QT_VERSION >= 0x051100
    QVariant path = index.siblingAtColumn(4).data();
    QVariant recording_flag = index.siblingAtColumn(5).data();
#else
    QVariant path = index.sibling(0,4).data();
    QVariant recording_flag = index.sibling(0,5).data();
#endif

    //qDebug() << path;
    //qDebug() << recording_flag;

    if (externalProgram.isEmpty()){
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setText(tr("EEG reader is not set"));
        msgBox.setInformativeText(tr("Do you want to set it now?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        int ret = msgBox.exec();

        if (ret == QMessageBox::Yes){
            chooseExternalProgram();
        }
        else{
            return;
        }
    }

    if (path.isValid()){ // in patient (where there is no path set) is not valid
        if (recording_flag.toBool()){
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("This is file is still being recorded, you will not able to review new data until you open it again."));
            msgBox.exec();
        }
        QStringList arguments;
        arguments << path.toString();
        QProcess *myProcess = new QProcess(nullptr);
        myProcess->start(this->externalProgram, arguments);
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



void MainWindow::chooseExternalProgram(){

    QString temp = QFileDialog::getOpenFileName(this, tr("Choose EEG reader"), defaultReaderFolder, tr("BrainLab reader (*.exe)"));

    if(temp.isEmpty()){
        return;
    }
    else{
        externalProgram = temp;
    }
};

void MainWindow::refreshDynamic(){
    if(!dynamic_dirs.isEmpty()){
        for (int j = 0; j < dynamic_dirs.size(); j++ ) {
            loadData(dynamic_dirs.at(j));
        }
    }
    updatePatientTreeModel();
    //buildTreeView(); // or rebuilt it somehow? What is more time consuming? Building model and treeview or mymap?
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
    qDebug() << QString("Compiled with Qt Version %1").arg(QT_VERSION_STR);

    QMessageBox messagewindow;
    messagewindow.setIcon(QMessageBox::NoIcon);
    messagewindow.setText("About this program");
    QString aboutQString = QString("EEGle Nest is a BrainLab record database using convertSIGtoEDF from Frederik-D-Weber to read BrainLab EEG files header.\n"
    "\n"
    "Built using Qt Creator 4.14.1 and Qt %1 (MSVC 2019, 64 bit)"
    "\n"
    "\n"
    "by Adam Kalina, Department of Neurology, Second Faculty of Medicine, Charles University and Motol University Hospital, 2021, during COVID-19").arg(QT_VERSION_STR);
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
    filemenu->addAction(tr("Refresh Dynamic"), this, SLOT(refreshDynamic()));
    filemenu->addAction(tr("Refresh Static"), this, SLOT(refreshStatic()));

    // ======== Settings ========
    setmenu = new QMenu(this);
    setmenu->setTitle(tr("&Settings"));
    setmenu->addAction(tr("Add Reader"), this, SLOT(chooseExternalProgram()));
    setmenu->addAction(tr("Folder list"), this, SLOT(notYetReady()));

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


    // ====== APPLICATION CYCLE ======
    readSettings(); // read setting from .ini file
    buildFilterLine(); // build filter line - do it first if you want it on the top
    initLoadData(); //load data and update patientMap

    if (patientMap.size() == 0){
        showNoFileWarning(); // show warning that there are no files to load
    }

}

// ======== Write and Read Settings ========

void MainWindow::writeSettings()
{
    //QSettings settings("PuffinSoft", "EEGle Nest");
    QSettings settings("settings.ini",QSettings::IniFormat);
    settings.setValue("external_program", externalProgram);
    settings.setValue("path2QMap",QMapFile);
    settings.setValue("defaultDataFolder","D:/Dropbox/Scripts/Cpp/");
    settings.setValue("defaultReaderFolder","D:/Dropbox/Scripts/Cpp/EEGLE/build-EEGle-Desktop_Qt_5_15_2_MinGW_64_bit-Release/");

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
    externalProgram = settings.value("external_program").toString();
    QMapFile = settings.value("path2QMap").toString();
    defaultDataFolder = settings.value("defaultDataFolder").toString();
    defaultReaderFolder = settings.value("defaultReaderFolder").toString();

    // check for duplicates here? Or will suffice in "Add folder dialog"

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

    qDebug() << "static directories: " << dynamic_dirs;
    qDebug() << "dynamic directories: " << static_dirs;

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
    out.setVersion(QDataStream::Qt_5_3);
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

    in.setVersion(QDataStream::Qt_5_3);

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

