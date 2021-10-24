#include "mainwindow.h"

void Patient::set_values (Record record) {
    id = record.id;
    name = record.name;
    sex = record.sex;
    no = 1;
    last_record = record.record_start;
    //records.push_back(record);
    records_map.insert(std::pair<string,Record>(record.file_name,record));
};

void Patient::add_record(Record record){
    // TO DO - check for duplicates?
    //records.push_back(record);
    records_map.insert(std::pair<string,Record>(record.file_name,record));

    // compares start of recordings and uses later
    if (difftime(last_record, record.record_start) < 0){
        last_record = record.record_start;
    };

    no = records_map.size(); // beter than no++ since it would increment even in duplicate files
};

string MainWindow::convert_time_for_sorting(const time_t * timer){ //deprecated
    // convert time
    char buffer [80];
    struct tm * timeinfo = localtime(timer);
    strftime (buffer,sizeof(buffer),"%Y-%m-%d %H:%M:%S",timeinfo);
    //qDebug() << buffer;
    return buffer;
}

void addPatient2model(QAbstractItemModel *model, string ID, Patient patient){

    // define patient
    model->insertRow(0);

    model->setData(model->index(0, 0), QString::fromStdString(ID));
    model->setData(model->index(0, 1), QString::fromLocal8Bit(patient.name.c_str()));
    model->setData(model->index(0, 2), QDateTime::fromSecsSinceEpoch(patient.last_record));
    model->setData(model->index(0, 3), patient.no, Qt::DisplayRole);
    model->setData(model->index(0, 3), Qt::AlignCenter, Qt::TextAlignmentRole);
    //QDateTime last_record = QDateTime::fromSecsSinceEpoch(patient.last_record); // This function was introduced in Qt 5.8., before that use QDateTime::fromTime_t

    const QModelIndex parent = model->index(0,0); // get the item in the first row and first column

    // iterate through records
    int ind = 0;
    int ncol = 6;
    model->insertColumns(0, ncol, parent); // adds a child to the previous item

    std::map<string, Record>::iterator to = patient.records_map.begin();
    for (patient.records_map.begin();to!=patient.records_map.end(); ++to){
        model->insertRows(ind, 1, parent); // adds a child to the previous item
        model->setData(model->index(ind, 0, parent), QString::fromStdString(to->first), Qt::DisplayRole);
        model->setData(model->index(ind, 1, parent), QString::fromLocal8Bit(to->second.class_code.c_str()), Qt::DisplayRole);
        model->setData(model->index(ind, 2, parent), QDateTime::fromSecsSinceEpoch(to->second.record_start), Qt::DisplayRole);
        //model->setData(model->index(0, 3, parent), "", Qt::DisplayRole);
        model->setData(model->index(ind, 4, parent), QString::fromLocal8Bit(to->second.file_path.c_str()), Qt::DisplayRole);
        model->setData(model->index(ind, 5, parent), to->second.recording_flag, Qt::DisplayRole);

        // if recording flag = 1 --> color the whole row read
        // TO DO - use delegate?
        if (to->second.recording_flag){
            for (int i = 0;i < ncol;i++){
                model->setData(model->index(ind, i, parent), QColor(Qt::red), Qt::ForegroundRole);
            }
        }

        ind++;
    }
}


QAbstractItemModel *createPatientTreeModel(QObject *parent, std::map<string, Patient> mymap)
{
    QStandardItemModel *model = new QStandardItemModel(0, 6, parent);

    model->setHeaderData(0, Qt::Horizontal, QString::fromLocal8Bit("Rodné èíslo"));
    model->setHeaderData(1, Qt::Horizontal, QString::fromLocal8Bit("Jméno"));
    model->setHeaderData(2, Qt::Horizontal, QString::fromLocal8Bit("Poslední EEG"));
    model->setHeaderData(3, Qt::Horizontal, QString::fromLocal8Bit("Poèet EEG"));
    model->setHeaderData(4, Qt::Horizontal, QString::fromLocal8Bit("Cesta"));
    model->setHeaderData(5, Qt::Horizontal, QString::fromLocal8Bit("Recorded"));
    // https://forum.qt.io/topic/123358/qtreeview-header-text-alignment/2

    // iterate over patients
    std::map<string, Patient>::iterator it = mymap.begin();
    for (it=mymap.begin(); it!=mymap.end(); ++it){
        //qDebug() << QString::fromLocal8Bit(it->first.c_str());
        addPatient2model(model,it->first, it->second);
    }

    return model;
}

void MainWindow::updatePatientTreeModel(){

    // to use it this proberly I would need to subclass QAbstractItemModel and define reset
    //model->beginResetModel();

    sourceModel->removeRows(0,sourceModel->rowCount());

    std::map<string, Patient>::iterator it = mymap.begin();
    for (it=mymap.begin(); it!=mymap.end(); ++it){
        //qDebug() << QString::fromLocal8Bit(it->first.c_str());
        addPatient2model(sourceModel,it->first, it->second);
    }

}

void MainWindow::buildTreeView(){

//    //set the layout
//    // TO DO - make the layour it more generic, even for error message
//    QWidget *centralWidget = new QWidget(this);
//    setCentralWidget(centralWidget);

//    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

//    // ======== FILTER ========
//    // TO DO - move this to separate function
//    // text line for filtering
//    QLineEdit *filter = new QLineEdit(centralWidget);
//    filter->setPlaceholderText(QString::fromLocal8Bit("jméno, rodné èíslo, datum yyyy-mm-dd"));
//    filter->setClearButtonEnabled(1);
//    //connect(filter, &QLineEdit::textChanged, this, SLOT(filter_text_changed));
//    connect(filter, SIGNAL(textChanged(QString)), this, SLOT(filter_text_changed(QString)));
//    layout->addWidget(filter);


    //QBrush *ligh_grey_brush = new QBrush(QColor(240,240,240));
    //QBrush ligh_grey_brush(QColor(240,240,240));

    // ======== TREE VIEW ========

    //QTreeView *proxyView = new QTreeView;

    delete treeView;

    treeView = new QTreeView;

    qDebug() << "creating source model";
    sourceModel = createPatientTreeModel(this, mymap); // create sourceModel

    //proxyModel = new QSortFilterProxyModel(this);
    proxyModel = new LeafFilterProxyModel(this); // use this custom FilterProxyModel

    proxyModel->setSourceModel(sourceModel);
    proxyModel->setRecursiveFilteringEnabled(1); // This property was introduced in Qt 5.10.
    proxyModel->setFilterKeyColumn(-1); // filter through all columns
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    treeView->setModel(proxyModel); // or set SourceModel here for no filtering
    treeView->setColumnHidden(4,1); // hide path to EEG file
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
    //treeView->setIndentation(20);
    connect(treeView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(double_click_tree(QModelIndex)));

    //treeView->setItemDelegate()

    layout->addWidget(treeView);
    // proxyView->setModel(proxyModel);
    //layout->addWidget(proxyView);

    //qDebug() << treeView->sizeHint();
};

void MainWindow::double_click_tree(QModelIndex index){
    //qDebug() << index.data(); // data pod kurzorem
    QVariant path = index.siblingAtColumn(4).data();
    QVariant recording_flag = index.siblingAtColumn(5).data();
    //qDebug() << path;
    //qDebug() << recording_flag;

    if (externalProgram.isEmpty()){
        QMessageBox msgBox;
        msgBox.setText("EEG reader is not set");
        msgBox.exec();
    }

    if (path.isValid()){ // in patient (where there is no path set) is not valid
        if (recording_flag.toBool()){
            QMessageBox msgBox;
            msgBox.setText("This is file is still being recorded, you will not able to review new data until you open it again.");
            msgBox.exec();
        }
        QStringList arguments;
        arguments << path.toString();
        QProcess *myProcess = new QProcess(nullptr);
        myProcess->start(this->externalProgram, arguments);
    }
}

// ======== LOAD DATA ========

void MainWindow::AddStaticFolderDialog(){
    AddFolderDialog("static");
}

void MainWindow::AddDynamicFolderDialog(){
    AddFolderDialog("dynamic");
}

void MainWindow::AddFolderDialog(QString folder_type){

    qDebug() << "here";

    new_dir = QFileDialog::getExistingDirectory(this, tr("Choose directory"), "D:/Dropbox/Scripts/Cpp/");
    if(new_dir.isEmpty()){
        return;
    }
    //qDebug() << new_dir;

    // add the folder only when it is not in the list already

    QStringList *dirs;

    if (folder_type == "static"){
        dirs = &static_dirs;
    }

    if(folder_type == "dynamic"){
        dirs = &dynamic_dirs;
    }

    if (!dirs->contains(new_dir)){
        *dirs << new_dir;
    }
    loadData(new_dir);
    updatePatientTreeModel();
    //buildTreeView();
};



void MainWindow::chooseExternalProgram(){

    externalProgram = QFileDialog::getOpenFileName(this, tr("Choose EEG reader"), "D:/Dropbox/Scripts/Cpp/EEGLE/build-EEGle-Desktop_Qt_5_15_2_MinGW_64_bit-Release/", tr("BrainLab reader (*.exe)"));

    if(externalProgram.isEmpty()){
        return;
    }
};

void MainWindow::filter_text_changed(const QString & text){
    //qDebug() << text;
    proxyModel->setFilterFixedString(text);
}

void MainWindow::loadData(QString path2load){

    //qDebug() << path2load;
    // TO DO
    // store the mymap on HDD or use SQLite
    // add only new files - probably best secured by MyMap
    // report duplicates
    // dynamic folders
    // check if the files still exists?

    //    // test - go throught the QDirIterator once and count files, then use it for qprogressdialog
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

        // check if .LOG file with same name exists - and if it does, flag the record as still being recorded
        // TO DO - is this the best way? There is lot of data in the header of recorded file, search for "TEMP" instead?

        QFileInfo fi(Qpath);
        record.recording_flag = QFileInfo::exists(fi.canonicalPath() + "/" + fi.baseName() + ".LOG"); // bool to int

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
        for (int j = 0; j < static_dirs.size(); ++j) {
            loadData(static_dirs.at(j));
        }
        for (int j = 0; j < dynamic_dirs.size(); j++ ) {
            loadData(dynamic_dirs.at(j));
        }
        qDebug() << "total no of files processed: " << no_files_loaded;
        buildTreeView();
    }

}

void MainWindow::refreshDynamic(){
    if(!dynamic_dirs.isEmpty()){
        for (int j = 0; j < dynamic_dirs.size(); j++ ) {
            loadData(dynamic_dirs.at(j));
        }
    }
    updatePatientTreeModel();
    //buildTreeView(); // or rebuilt it somehow? What is more time consuming? Building model and treeview or mymap?
}

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

void MainWindow::buildFilterLine(){
    // ======== FILTER ========
    // TO DO - move this to separate function
    // text line for filtering
    filter = new QLineEdit(centralWidget);
    filter->setPlaceholderText(QString::fromLocal8Bit("jméno, rodné èíslo, datum yyyy-mm-dd"));
    filter->setClearButtonEnabled(1);
    //connect(filter, &QLineEdit::textChanged, this, SLOT(filter_text_changed));
    connect(filter, SIGNAL(textChanged(QString)), this, SLOT(filter_text_changed(QString)));
    layout->addWidget(filter);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{


    // ======== Main Menu ========
    menubar = menuBar();

    // ======== File menu ========
    filemenu = new QMenu(this);
    filemenu->setTitle("&Data");
    filemenu->addAction("Add Static Folder", this, SLOT(AddStaticFolderDialog()));
    filemenu->addAction("Add Dynamic Folder", this, SLOT(AddDynamicFolderDialog()));
    filemenu->addAction("Add Reader", this, SLOT(chooseExternalProgram()));
    filemenu->addAction("Refresh Dynamic", this, SLOT(refreshDynamic()));


    // TO DO
    // delete setting
    // help
    // folder list


    menubar->addMenu(filemenu);

    // ====== LAYOUT =====

    //set the layout
    // TO DO - make the layour it more generic, even for error message
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    layout = new QVBoxLayout(centralWidget);


    // ====== APPLICATION CYCLE ======
    readSettings(); // read setting from .ini file
    buildFilterLine(); // build filter line - do it first if you want it on the top
    initLoadData(); //load data and update mymap

    if (mymap.size() == 0){
        showNoFileWarning(); // show warning that where are no filed to load
    }

}

// ======== Write and Read Settings ========

void MainWindow::writeSettings()
{
    //QSettings settings("PuffinSoft", "EEGle Nest");
    QSettings settings("settings.ini",QSettings::IniFormat);
    settings.setValue("external_program", externalProgram);

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


MainWindow::~MainWindow()
{
    writeSettings(); //save setting in .ini file
}

