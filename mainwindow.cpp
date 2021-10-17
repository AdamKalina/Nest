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

    no++; // increment the number of recordings
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


void MainWindow::click_test(QTreeWidgetItem* Item){

    QVariant path = Item->data(4,0);
    // TO DO - continue only when record is clicked (not patient)
    if (path.isValid()){
        //qDebug() << Item->data(4,0);
        QStringList arguments;
        arguments << path.toString();
        QProcess *myProcess = new QProcess(nullptr);
        myProcess->start(this->externalProgram, arguments);
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{

    // TO DO - move this to separate class
    // implement for multiple folders
    // store the mymap on HDD or use SQLite
    // add only new files
    // display files being recorded and make them unable to open
    //QDir directory("D:/Dropbox/Scripts/Cpp/data_test");
    //QFileInfoList sigs = directory.entryInfoList(QStringList() << "*.sig" << "*.SIG",QDir::Files);


    int ii = 0;

    // QDirIterot - go through files recursively
    QDirIterator QDit(QString("D:/Dropbox/Scripts/Cpp/data_test"), QStringList() << "*.sig" << "*.SIG", QDir::Files, QDirIterator::Subdirectories);
    while (QDit.hasNext()){

        string path= QDit.next().toLocal8Bit().data();

        ii++;
        //qDebug() << ii << QDit.filePath();


        // this function returns only the data needed - maybe rename it
        Record record = read_signal_file(path);

        std::map<string, Patient>::iterator it;
        it = mymap.find(record.id);
        if (it != mymap.end()){
            //mymap.erase (it);
            //cout << it->first << " already exists" << endl;
            it->second.add_record(record);
        }
        else{
            Patient patient;
            patient.set_values(record);
            // insert into map
            mymap.insert(std::pair<string,Patient>(patient.id,patient));
        }
    }

    qDebug() << "no of files being processed: " << ii;

    // TO DO
    // formatting - make it look nicer
    // filterting and custom ordering (order by no of record)
    // https://doc.qt.io/qt-5/qsortfilterproxymodel.html#details
    // make separate constructors for items

    QBrush *ligh_grey_brush = new QBrush(QColor(240,240,240));

    // define the main TreeWidget
    QTreeWidget *treeWidget = new QTreeWidget();
    setCentralWidget(treeWidget);
    treeWidget->setColumnCount(4);
    treeWidget->setSortingEnabled(1);
    treeWidget->sortByColumn(0,Qt::DescendingOrder);

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
        patientItem ->setBackground(0,*ligh_grey_brush);
        patientItem ->setBackground(1,*ligh_grey_brush);
        patientItem ->setBackground(2,*ligh_grey_brush);
        patientItem ->setBackground(3,*ligh_grey_brush);

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
    //connect(treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(click_test(QTreeWidgetItem*)));
    connect(treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(click_test(QTreeWidgetItem*)));
    treeWidget->setColumnHidden(4,1); //hide file path
    treeWidget->show();

}

MainWindow::~MainWindow()
{

}

//void MainWindow::paintEvent(QPaintEvent *event)
//{

//    //Drawing Texts
//    int x = 20;
//    int y = 30;
//    int inc = 30;
//    int i = 1;
//    //QTextCodec *codec = QTextCodec::codecForName("Windows-1250");

//    QPainter mytext(this);
//    mytext.setFont(QFont("Times", 16, QFont::Bold));
//    mytext.drawText(QPoint(x,y), "My map contains");


//    mytext.setFont(QFont("Times", 16, QFont::Normal));
//    std::map<string, Patient>::iterator it = mymap.begin();
//    for (it=mymap.begin(); it!=mymap.end(); ++it){

//        mytext.drawText(QPoint(x,y+i*inc), it->first.data());

//        //QString q = QString::fromLocal8Bit(it->second.name.c_str());

//        mytext.drawText(QPoint(x+200,y+i*inc), QString::fromLocal8Bit(it->second.name.c_str()));
//        QString no = QString::number(it->second.no);
//        mytext.drawText(QPoint(x+550,y+i*inc), QString("no of records: %1").arg(no));
//        i++;

//        std::map<string, Record>::iterator to = it->second.records_map.begin();
//        for (it->second.records_map.begin();to!=it->second.records_map.end(); ++to){
//            mytext.drawText(QPoint(x+200,y+i*inc), QString::fromStdString(to->first));
//            mytext.drawText(QPoint(x+550,y+i*inc), QString::fromLocal8Bit(to->second.class_code.c_str()));
//            mytext.drawText(QPoint(x+700,y+i*inc), QString("recorded on: %1").arg(ctime(&to->second.record_start)));
//            i++;
//        }
//    };


//    // Giving Style To Texts
//    //QTextDocument document;
//    //QRect rect(0,0,250,250 );
//    //mytext.translate(100,50);

//    //document.setHtml("<b>Hello</b><font color='red' size='30'>Qt5 C++ </font>");
//    //document.drawContents(&mytext, rect);
//}

