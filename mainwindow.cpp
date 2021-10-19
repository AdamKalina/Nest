#include "mainwindow.h"

//std::map<string, Patient> mymap;

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

    no = records_map.size();

    //no++; // increment the number of recordings
};

string MainWindow::convert_time_for_sorting(const time_t * timer){
    // convert time
    char buffer [80];
    struct tm * timeinfo = localtime(timer);
    strftime (buffer,sizeof(buffer),"%Y-%m-%d %H:%M:%S",timeinfo);
    //qDebug() << buffer;
    return buffer;
    ;
}


void MainWindow::double_click_record(QTreeWidgetItem* Item){

    QVariant path = Item->data(4,0);
    // TO DO - continue only when record is clicked (not patient)
    if (externalProgram.isEmpty()){
        QMessageBox msgBox;
        msgBox.setText("EEG reader is not set");
        msgBox.exec();
    }

    if (path.isValid()){
        //qDebug() << Item->data(4,0);
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

    if(stat_dir.isEmpty()){ // if there is no path to data set it will ask for it righ away
        AddFolderDialog();
    }

    int ii = 0;

    // QDirIterator - go through files recursively

    QDirIterator QDit(this->stat_dir, QStringList() << "*.sig" << "*.SIG", QDir::Files, QDirIterator::Subdirectories);
    while (QDit.hasNext()){

        string path= QDit.next().toLocal8Bit().data();

        ii++;
        //qDebug() << ii << QDit.filePath();


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
    buildTreeWidget();
}

void MainWindow::buildTreeWidget(){

    //set the layout
    // TO DO - make it more generic, even for error message
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    // is it possible to stretch the main window to fit the tree? Probably not https://www.qtcentre.org/threads/53948-resize-to-content-of-a-QTreeWidget


    QLineEdit *filter = new QLineEdit(centralWidget);
    filter->setPlaceholderText("filter");
    filter->setClearButtonEnabled(1);
    //connect(filter, &QLineEdit::textChanged, this, SLOT(filter_text_changed));
    connect(filter, SIGNAL(textChanged(QString)), this, SLOT(filter_text_changed(QString)));
    layout->addWidget(filter);

    // TO DO
    // formatting - make it look nicer
    // filterting and custom ordering (order by no of record)
    // https://doc.qt.io/qt-5/qsortfilterproxymodel.html#details
    // make separate constructors for items

    //QBrush *ligh_grey_brush = new QBrush(QColor(240,240,240));
    QBrush ligh_grey_brush(QColor(240,240,240));

    // define filter
    //MyItemModel *sourceModel = new MyItemModel(this);
    //QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);

    //proxyModel->setSourceModel(sourceModel);
    //treeView->setModel(proxyModel);


    // define the main TreeWidget
    QTreeWidget *treeWidget = new QTreeWidget();
    layout->addWidget(treeWidget);
    //setCentralWidget(treeWidget);
    treeWidget->setColumnCount(4);
    treeWidget->setSortingEnabled(1);
    treeWidget->sortByColumn(2,Qt::DescendingOrder); //newest files first
    //treeWidget->setModel(sourceModel);


    // Header of main TreeWidget
    QStringList labels;
    labels << QString::fromLocal8Bit("ID") << QString::fromLocal8Bit("Jméno") << QString::fromLocal8Bit("Poslední EEG") << QString::fromLocal8Bit("Poèet EEG") << QString("path");
    treeWidget->setHeaderLabels(labels);
    treeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    treeWidget->header()->setDefaultAlignment(Qt::AlignCenter);

    QList<QTreeWidgetItem *> items;

    // iterate over patients
    std::map<string, Patient>::iterator it = mymap.begin();
    for (it=mymap.begin(); it!=mymap.end(); ++it){

        QStringList patientItemLabel;
        string ftime_last = convert_time_for_sorting(&it->second.last_record);
        patientItemLabel << QStringList(it->first.data()) << QString::fromLocal8Bit(it->second.name.c_str()) << QString::fromStdString(ftime_last) << QString::number(it->second.no);
        QTreeWidgetItem *patientItem = new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr), patientItemLabel);

        // TO DO - set this at once
        patientItem ->setBackground(0,ligh_grey_brush);
        patientItem ->setBackground(1,ligh_grey_brush);
        patientItem ->setBackground(2,ligh_grey_brush);
        patientItem ->setBackground(3,ligh_grey_brush);

        // iterate over records
        std::map<string, Record>::iterator to = it->second.records_map.begin();
        for (it->second.records_map.begin();to!=it->second.records_map.end(); ++to){
            QStringList recordItemLabel;

            // prepare time for display
            string ftime = convert_time_for_sorting(&to->second.record_start);

            // construct ItemLabel
            recordItemLabel << QString::fromStdString(to->first);
            recordItemLabel << QString::fromLocal8Bit(to->second.class_code.c_str());
            recordItemLabel << QString::fromStdString(ftime) << QString::fromStdString("") << QString::fromStdString(to->second.file_path);

            QTreeWidgetItem *recordItem = new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr), recordItemLabel);

            patientItem->addChild(recordItem);
            patientItem->setTextAlignment(3,4); // alignment must be set on the parent (= patient)
            items.append(patientItem);
        }

    }

    treeWidget->insertTopLevelItems(0, items);
    connect(treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(double_click_record(QTreeWidgetItem*)));
    treeWidget->setColumnHidden(4,1); //hide file path
    treeWidget->show();
};

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
        buildTreeWidget(); // do the thing
    }

}

MainWindow::~MainWindow()
{
    writeSettings(); //save setting in .ini file
}

