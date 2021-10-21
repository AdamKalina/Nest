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

    no = records_map.size(); // beter than no++ since it woul increment even in duplicate files
};

void addPatient2model(QAbstractItemModel *model, string ID, Patient patient){

    // define patient
    model->insertRow(0);
    model->setData(model->index(0, 0), QString::fromStdString(ID));
    model->setData(model->index(0, 1), QString::fromLocal8Bit(patient.name.c_str()));
    model->setData(model->index(0, 2), QDateTime::fromSecsSinceEpoch(patient.last_record));
    model->setData(model->index(0, 3), patient.no, Qt::DisplayRole);
    //QDateTime last_record = QDateTime::fromSecsSinceEpoch(patient.last_record); // This function was introduced in Qt 5.8., before that use QDateTime::fromTime_t


    const QModelIndex parent = model->index(0,0); // get the item in the first row and first column

    // iterate through records
    int ind = 0;
    model->insertColumns(0, 5, parent); // adds a child to the previous item

    // iterate over records
    std::map<string, Record>::iterator to = patient.records_map.begin();
    for (patient.records_map.begin();to!=patient.records_map.end(); ++to){
        model->insertRows(ind, 1, parent); // adds a child to the previous item
        model->setData(model->index(ind, 0, parent), QString::fromStdString(to->first), Qt::DisplayRole);
        model->setData(model->index(ind, 1, parent), QString::fromLocal8Bit(to->second.class_code.c_str()), Qt::DisplayRole);
        model->setData(model->index(ind, 2, parent), QDateTime::fromSecsSinceEpoch(to->second.record_start), Qt::DisplayRole);
        //model->setData(model->index(0, 3, parent), "", Qt::DisplayRole);
        model->setData(model->index(ind, 4, parent), QString::fromLocal8Bit(to->second.file_path.c_str()), Qt::DisplayRole);
        ind++;
    }
}


QAbstractItemModel *createPatientTreeModel(QObject *parent, std::map<string, Patient> mymap)
{
    QStandardItemModel *model = new QStandardItemModel(0, 5, parent);

    model->setHeaderData(0, Qt::Horizontal, QString::fromLocal8Bit("Rodné èíslo"));
    model->setHeaderData(1, Qt::Horizontal, QString::fromLocal8Bit("Jméno"));
    model->setHeaderData(2, Qt::Horizontal, QString::fromLocal8Bit("Poslední EEG"));
    model->setHeaderData(3, Qt::Horizontal, QString::fromLocal8Bit("Poèet EEG"));
    model->setHeaderData(4, Qt::Horizontal, QString::fromLocal8Bit("Cesta"));

    // https://forum.qt.io/topic/123358/qtreeview-header-text-alignment/2

    // iterate over patients
    std::map<string, Patient>::iterator it = mymap.begin();
    for (it=mymap.begin(); it!=mymap.end(); ++it){
        addPatient2model(model,it->first, it->second);
    }

    return model;
}

void MainWindow::buildTreeView(){

    //set the layout
    // TO DO - make it more generic, even for error message
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    // is it possible to stretch the main window to fit the tree? Probably not https://www.qtcentre.org/threads/53948-resize-to-content-of-a-QTreeWidget


    // TO DO - move this to separate function
    // text line for filtering
    QLineEdit *filter = new QLineEdit(centralWidget);
    filter->setPlaceholderText("filter");
    filter->setClearButtonEnabled(1);
    //connect(filter, &QLineEdit::textChanged, this, SLOT(filter_text_changed));
    connect(filter, SIGNAL(textChanged(QString)), this, SLOT(filter_text_changed(QString)));
    layout->addWidget(filter);

    // TO DO
    // formatting - make it look nicer

    //QBrush *ligh_grey_brush = new QBrush(QColor(240,240,240));
    QBrush ligh_grey_brush(QColor(240,240,240));

    // ======== TREE VIEW ========

    QTreeView *sourceView = new QTreeView;
    sourceView->setModel(createPatientTreeModel(this, mymap));
    sourceView->setColumnHidden(4,1); // hide path to EEG file
    sourceView->setSortingEnabled(1);
    sourceView->sortByColumn(2,Qt::DescendingOrder); //newest files first
    sourceView->header()->setSectionsMovable(0); //disable moving columns by dragging
    sourceView->header()->setDefaultAlignment(Qt::AlignCenter); //align header labels to center
    sourceView->header()->setStretchLastSection(false);
    sourceView->header()->setFont(QFont("Sans Serif", 12, QFont::Bold));
    sourceView->setEditTriggers(QAbstractItemView::NoEditTriggers); // turn off editing of existin data by user
    sourceView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    sourceView->setAlternatingRowColors(1);
    //sourceView->setIndentation(20);
    connect(sourceView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(double_click_tree(QModelIndex)));
    layout->addWidget(sourceView);
};

string MainWindow::convert_time_for_sorting(const time_t * timer){ //deprecated
    // convert time
    char buffer [80];
    struct tm * timeinfo = localtime(timer);
    strftime (buffer,sizeof(buffer),"%Y-%m-%d %H:%M:%S",timeinfo);
    //qDebug() << buffer;
    return buffer;
}


void MainWindow::double_click_tree(QModelIndex index){
    //qDebug() << index.data(); // data pod kurzorem
    QVariant path = index.siblingAtColumn(4).data();
    //qDebug() << path;

    if (externalProgram.isEmpty()){
        QMessageBox msgBox;
        msgBox.setText("EEG reader is not set");
        msgBox.exec();
    }

    if (path.isValid()){ // in patient (where there is no path set) is not valid
        QStringList arguments;
        arguments << path.toString();
        QProcess *myProcess = new QProcess(nullptr);
        myProcess->start(this->externalProgram, arguments);
    }

}

void MainWindow::AddFolderDialog(){

    stat_dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), "D:/Dropbox/Scripts/Cpp/");
    if(stat_dir.isEmpty()){
        return;
    }
    qDebug() << stat_dir;
    loadData();
};


void MainWindow::chooseExternalProgram(){

    externalProgram = QFileDialog::getOpenFileName(this, tr("Choose EEG reader"), "D:/Dropbox/Scripts/Cpp/EEGLE/build-EEGle-Desktop_Qt_5_15_2_MinGW_64_bit-Release/", tr("BrainLab reader (*.exe)"));

    if(externalProgram.isEmpty()){
        return;
    }
};

void MainWindow::filter_text_changed(const QString & text){
    qDebug() << text;
}


void MainWindow::writeSettings()
{
    //QSettings settings("PuffinSoft", "EEGle Nest");
    QSettings settings("settings.ini",QSettings::IniFormat);
    settings.setValue("external_program", externalProgram);
    settings.setValue("stat_dir",stat_dir);
}

void MainWindow::readSettings()
{
    QSettings settings("settings.ini",QSettings::IniFormat);
    stat_dir = settings.value("stat_dir").toString();
    externalProgram = settings.value("external_program").toString();
}

void MainWindow::loadData(){
    // TO DO
    // implement for multiple folders - problematic since settings can not store QStringList - adding one folder at once will be needed
    // store the mymap on HDD or use SQLite
    // add only new files - probably best secured by MyMap
    // display files being recorded and make them unable to open

    if(stat_dir.isEmpty()){ // if there is no path to data set it will ask for it right away
        AddFolderDialog();
    }

    int ii = 0;

    // QDirIterator - go through files recursively

    QDirIterator QDit(this->stat_dir, QStringList() << "*.sig" << "*.SIG", QDir::Files, QDirIterator::Subdirectories);
    while (QDit.hasNext()){

        string path= QDit.next().toLocal8Bit().data();

        ii++;

        // this function returns only the data needed - maybe rename it
        Record record = read_signal_file(path);

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

    qDebug() << "no of files being processed: " << ii;
    buildTreeView();
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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{


    // ======== Main Menu ========
    menubar = menuBar();

    // ======== File menu ========
    filemenu = new QMenu(this);
    filemenu->setTitle("&Data");
    filemenu->addAction("Add Folder", this, SLOT(AddFolderDialog()));
    filemenu->addAction("Add Reader", this, SLOT(chooseExternalProgram()));
    // TO DO
    // add dynamic folder + refresh button
    // delete setting
    menubar->addMenu(filemenu);

    readSettings(); // read setting from .ini file
    loadData(); //load data and update mymap


    if (mymap.size() == 0){
        showNoFileWarning(); // show warning that where are no filed to load
    }
    else{
        buildTreeView(); // do the thing
    }

}

MainWindow::~MainWindow()
{
    writeSettings(); //save setting in .ini file
}

