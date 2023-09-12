#include "options_dialog.h"


OptionsDialog::OptionsDialog(QWidget *w_parent)
{
    mainwindow = (MainWindow *)w_parent;

    optionsdialog = new QDialog(w_parent);

    loadSettingsFromMainWindow();

    QFont b("Arial", 10, QFont::Bold);
    QFont ff("Arial", 10);
    QFont ii("Arial", 10, QFont::StyleItalic);
    ii.setItalic(true);

    optionsdialog->setWindowTitle("Options");
    optionsdialog->setModal(true);
    optionsdialog->setAttribute(Qt::WA_DeleteOnClose, true);
    optionsdialog->setSizeGripEnabled(true);

    tabholder = new QTabWidget;

    /////////////////////////////////////// tab 1 Refresh settings ///////////////////////////////////////////////////////////////////////

    tab1 = new QWidget;

    //======== FILES SETTINGS ========
    QLabel *filesLabel = new QLabel(tr("Files setting"));
    filesLabel->setFont(b);

    // Choose what files should be refreshed every time
    QCheckBox *refreshAll = new QCheckBox("Refresh all files in dynamic folders");
    refreshAll->setFont(ff);
    QCheckBox *refreshYear = new QCheckBox("Refresh only files modified in the last year");
    refreshYear->setFont(ff);
    QCheckBox *refreshMonth = new QCheckBox("Refresh only files modified in the last month");
    refreshMonth->setFont(ff);
    QCheckBox *refreshWeek = new QCheckBox("Refresh only files modified in the last week");
    refreshWeek->setFont(ff);
    QCheckBox *refreshToday = new QCheckBox("Refresh only files modified in the last 24 hours");
    refreshToday->setFont(ff);


    QButtonGroup *buttonGroup = new QButtonGroup( this );
    buttonGroup->addButton(refreshAll,0);
    buttonGroup->addButton(refreshYear,365);
    buttonGroup->addButton(refreshMonth,31);
    buttonGroup->addButton(refreshWeek,7);
    buttonGroup->addButton(refreshToday,1);
    buttonGroup->setExclusive(true);

    buttonGroup->button(un_options.periodicRefreshMode)->setChecked(1);

    QLabel *filesInfo = new QLabel(tr("To save time during refresh you can constrain which files are checked to files modified in given period."));
    filesInfo->setFont(ii);

    //======== PERIODIC REFRESHING SETTINGS ========
    QLabel *dialLabel = new QLabel(tr("Periodic refreshing setting"));
    dialLabel->setFont(b);

    // QDial
    hourDial = new QDial;
    hourDial->setMinimumSize(300,300);
    hourDial->setSingleStep(5); // small notches
    hourDial->setPageStep(15); // big notches
    hourDial->setMinimum(0);
    hourDial->setMaximum(60);

    int dialValue;

    if(mainwindow->nestOptions.refreshingPeriod > 30){
        dialValue = mainwindow->nestOptions.refreshingPeriod - 30;
    }else{
        dialValue = mainwindow->nestOptions.refreshingPeriod + 30;
    }
    hourDial->setValue(dialValue);
    hourDial->setWrapping(1);
    hourDial->setNotchesVisible(1);
    hourDial->setEnabled(mainwindow->nestOptions.periodicRefreshingEnabled);

    timeLabel = new QLabel(QString("Refresh automatically every %1 minutes").arg(mainwindow->nestOptions.refreshingPeriod)); // construct like this so it can be updated
    timeLabel->setFont(ff);
    timeLabel->setEnabled(mainwindow->nestOptions.periodicRefreshingEnabled);

    QCheckBox *usePeriodic = new QCheckBox("Use periodic refreshing");
    usePeriodic->setFont(ff);
    usePeriodic->setChecked(mainwindow->nestOptions.periodicRefreshingEnabled);
    QLabel *usePeriodicInfo = new QLabel(tr("If checked it will perform refresh of dynamic folders in interval set on the dial."));
    usePeriodicInfo->setFont(ii);

    useWorkingHours = new QCheckBox("Use in working hours only");
    useWorkingHours->setFont(ff);
    useWorkingHours->setEnabled(mainwindow->nestOptions.periodicRefreshingEnabled);
    useWorkingHours->setChecked(mainwindow->nestOptions.refreshWorkingHoursOnly);
    QLabel *useWorkingHoursInfo = new QLabel(tr("If checked it will perform periodic refresh in working hours only - set from 7 AM to 17 PM"));
    useWorkingHoursInfo->setFont(ii);

    // ======== REFRESH STATIC ========

    QLabel *staticLabel = new QLabel(tr("Static folders setting"));
    staticLabel->setFont(b);

    QCheckBox *loadStaticOnRefresh = new QCheckBox("Load static data on refresh");
    loadStaticOnRefresh->setFont(ff);
    loadStaticOnRefresh->setChecked(mainwindow->nestOptions.refreshLoadStatic);
    QLabel *staticLabelInfo = new QLabel(tr("If checked it will load data from refreshed static folders into model.\nIf unchecked it will only refresh database entries."));
    staticLabelInfo->setFont(ii);


    // ======== LAYOUT ORGANIZATION ========
    QVBoxLayout *bottomLeft1 = new QVBoxLayout;
    // bottomLeft->addStretch();
    //bottomLeft->setSpacing(4);
    bottomLeft1->addWidget(usePeriodic);
    bottomLeft1->addWidget(usePeriodicInfo);
    bottomLeft1->addSpacing(10);
    bottomLeft1->addWidget(useWorkingHours);
    bottomLeft1->addWidget(useWorkingHoursInfo);
    bottomLeft1->addSpacing(10);
    bottomLeft1->addWidget(staticLabel);
    bottomLeft1->addWidget(loadStaticOnRefresh);
    bottomLeft1->addWidget(staticLabelInfo);
    bottomLeft1->addStretch();

    QVBoxLayout *bottomRight = new QVBoxLayout;
    bottomRight->addWidget(hourDial);
    bottomRight->addWidget(timeLabel, Qt::AlignCenter);

    QHBoxLayout *bottom = new QHBoxLayout;
    bottom->addLayout(bottomLeft1);
    bottom->addLayout(bottomRight);

    QVBoxLayout *mainLayout1 = new QVBoxLayout;
    mainLayout1->addWidget(filesLabel);
    mainLayout1->addWidget(refreshAll);
    mainLayout1->addWidget(refreshYear);
    mainLayout1->addWidget(refreshMonth);
    mainLayout1->addWidget(refreshWeek);
    mainLayout1->addWidget(refreshToday);
    mainLayout1->addWidget(filesInfo);
    mainLayout1->addSpacing(20);
    //mainLayout->addSpacerItem()
    mainLayout1->addWidget(dialLabel);
    mainLayout1->addLayout(bottom);
    mainLayout1->addSpacing(20);
    tab1->setLayout(mainLayout1);

    /////////////////////////////////////// tab 2 External programs ///////////////////////////////////////////////////////////////////////
    tab2 = new QWidget;

    QLabel *brainlabReaderLabel = new QLabel(tr("Brainlab - EEG reader"));
    brainlabReaderLabel->setFont(b);
    brainlabReaderEdit = new QLineEdit(un_options.brainlabReader);

    QLabel *brainlabControlLabel = new QLabel(tr("Brainlab - EEG reader for files still being recorded (control)"));
    brainlabControlLabel->setFont(b);
    brainlabControlEdit = new QLineEdit(un_options.brainlabControl);

    QLabel *harmonieReaderlabel = new QLabel(tr("Harmonie - EEG reader"));
    harmonieReaderlabel->setFont(b);
    harmonieReaderEdit = new QLineEdit(un_options.harmonieReader);

    QLabel *dicomReaderLabel = new QLabel(tr("DICOM reader"));
    dicomReaderLabel->setFont(b);
    dicomReaderEdit = new QLineEdit(un_options.dicomReaderPath);
    dicomReaderEdit->setEnabled(mainwindow->nestOptions.dicomReaderEnable);

    AddBrainlabReaderButton = new QPushButton;
    AddBrainlabReaderButton->setText(tr("Add Brainlab reader"));
    AddBrainlabReaderButton->setIcon(mainwindow->style()->standardIcon(QStyle::SP_FileDialogNewFolder));

    AddBrainlabControlButton = new QPushButton;
    AddBrainlabControlButton->setText(tr("Add Brainlab control"));
    AddBrainlabControlButton->setIcon(mainwindow->style()->standardIcon(QStyle::SP_FileDialogNewFolder));

    AddHarmonieReaderButton = new QPushButton;
    AddHarmonieReaderButton->setText(tr("Add Harmonie reader"));
    AddHarmonieReaderButton->setIcon(mainwindow->style()->standardIcon(QStyle::SP_FileDialogNewFolder));

    QCheckBox *allowDicomCheckBox = new QCheckBox("Allow export");
    allowDicomCheckBox->setFont(ff);
    allowDicomCheckBox->setChecked(mainwindow->nestOptions.dicomReaderEnable);

    AddDicomReaderButton = new QPushButton;
    AddDicomReaderButton ->setText(tr("Add DICOM reader"));
    AddDicomReaderButton ->setIcon(mainwindow->style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    AddDicomReaderButton->setEnabled(mainwindow->nestOptions.dicomReaderEnable);

    QHBoxLayout *hlayout_reader = new QHBoxLayout;
    hlayout_reader->addWidget(brainlabReaderEdit);
    hlayout_reader->addWidget(AddBrainlabReaderButton);

    QHBoxLayout *hlayout_control = new QHBoxLayout;
    hlayout_control->addWidget(brainlabControlEdit);
    hlayout_control->addWidget(AddBrainlabControlButton);

    QHBoxLayout *hlayout_harmonie = new QHBoxLayout;
    hlayout_harmonie->addWidget(harmonieReaderEdit);
    hlayout_harmonie->addWidget(AddHarmonieReaderButton);

    QHBoxLayout *hlayout_dicom = new QHBoxLayout;
    hlayout_dicom->addWidget(dicomReaderEdit);
    hlayout_dicom->addWidget(AddDicomReaderButton);

    QVBoxLayout *vlayout2 = new QVBoxLayout;
    vlayout2->addWidget(brainlabReaderLabel,0,Qt::AlignBottom);
    vlayout2->addLayout(hlayout_reader);
    vlayout2->addSpacing(40);
    vlayout2->addWidget(brainlabControlLabel,0,Qt::AlignBottom);
    vlayout2->addLayout(hlayout_control);
    vlayout2->addSpacing(40);
    vlayout2->addWidget(harmonieReaderlabel,0,Qt::AlignBottom);
    vlayout2->addLayout(hlayout_harmonie);
    vlayout2->addSpacing(40);
    vlayout2->addWidget(allowDicomCheckBox);
    vlayout2->addWidget(dicomReaderLabel,0,Qt::AlignBottom);
    vlayout2->addLayout(hlayout_dicom);
    vlayout2->addStretch(10);

    tab2->setLayout(vlayout2);
    tab2->adjustSize();

    /////////////////////////////////////// tab 3 EDF export ///////////////////////////////////////////////////////////////////////
    tab3 = new QWidget;

    QCheckBox *allowExportCheckBox = new QCheckBox("Allow export");
    allowExportCheckBox->setFont(ff);
    allowExportCheckBox->setChecked(mainwindow->nestOptions.exportAllow);

    QLabel *exportLabel = new QLabel(tr("EDF exporter"));
    exportLabel->setFont(b);
    exportEdit = new QLineEdit(mainwindow->nestOptions.exportProgram);
    exportEdit->setEnabled(mainwindow->nestOptions.exportAllow);

    changeExportProgramButton = new QPushButton;
    changeExportProgramButton->setText(tr("Choose Export Program"));
    changeExportProgramButton->setIcon(mainwindow->style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    changeExportProgramButton->setEnabled(mainwindow->nestOptions.exportAllow);

    QLabel *exportPathLabel = new QLabel(tr("Path to exported files"));
    exportPathLabel->setFont(b);
    exportPathEdit = new QLineEdit(mainwindow->nestOptions.exportPath);
    exportPathEdit->setEnabled(mainwindow->nestOptions.exportAllow);

    changeExportPathButton = new QPushButton;
    changeExportPathButton->setText(tr("Choose export path"));
    changeExportPathButton->setIcon(mainwindow->style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    changeExportPathButton->setEnabled(mainwindow->nestOptions.exportAllow);

    anonymizeCheckBox = new QCheckBox(tr("Anonymize"));
    anonymizeCheckBox->setFont(ff);
    anonymizeCheckBox->setChecked(mainwindow->nestOptions.exportAnonymize);
    anonymizeCheckBox->setEnabled(mainwindow->nestOptions.exportAllow);

    shortenCheckBox = new QCheckBox(tr("Shorten event labels"));
    shortenCheckBox->setFont(ff);
    shortenCheckBox->setChecked(mainwindow->nestOptions.exportShortenLabels);
    shortenCheckBox->setEnabled(mainwindow->nestOptions.exportAllow);

    systemEventsCheckBox = new QCheckBox(tr("Enable export of system events"));
    systemEventsCheckBox->setFont(ff);
    systemEventsCheckBox->setChecked(mainwindow->nestOptions.exportSystemEvents);
    systemEventsCheckBox->setEnabled(mainwindow->nestOptions.exportAllow);

    enableDebugModeCheckBox = new QCheckBox(tr("Enable debug mode for export"));
    enableDebugModeCheckBox->setFont(ff);
    enableDebugModeCheckBox->setChecked(mainwindow->nestOptions.exportEnableDebug);
    enableDebugModeCheckBox->setEnabled(mainwindow->nestOptions.exportAllow);

    QHBoxLayout *hlayout_export_program = new QHBoxLayout;
    hlayout_export_program->addWidget(exportEdit);
    hlayout_export_program->addWidget(changeExportProgramButton);

    QHBoxLayout *hlayout_export_path = new QHBoxLayout;
    hlayout_export_path->addWidget(exportPathEdit);
    hlayout_export_path->addWidget(changeExportPathButton);

    QVBoxLayout *vlayout3 = new QVBoxLayout;
    vlayout3->addWidget(allowExportCheckBox);
    vlayout3->addWidget(exportLabel,0,Qt::AlignBottom);
    vlayout3->addLayout(hlayout_export_program);
    vlayout3->addSpacing(40);
    vlayout3->addWidget(exportPathLabel,0,Qt::AlignBottom);
    vlayout3->addLayout(hlayout_export_path);
    vlayout3->addSpacing(40);
    vlayout3->addWidget(anonymizeCheckBox);
    vlayout3->addWidget(shortenCheckBox);
    vlayout3->addWidget(systemEventsCheckBox);
    vlayout3->addWidget(enableDebugModeCheckBox);
    vlayout3->addStretch(10);

    tab3->setLayout(vlayout3);
    tab3->adjustSize();

    /////////////////////////////////////// tab 4 Other ///////////////////////////////////////////////////////////////////////
    tab4 = new QWidget;

    QCheckBox *allowDeleteRecordCheckBox = new QCheckBox("Allow database editing by user");
    allowDeleteRecordCheckBox->setFont(ff);
    allowDeleteRecordCheckBox->setChecked(mainwindow->nestOptions.recordDeleteAllow);

    QLabel *deleteRecordInfo = new QLabel(tr("When enabled, use right click on record to delete it. If it is the last record, it deletes the patient as well.\n"
"On refresh it loads the data again, so be sure you correct the file info first."));
    deleteRecordInfo->setFont(ii);

    QLabel *months2loadSpinBoxInfo = new QLabel(tr("Number of months (of patients with EEG) to load into Nest during initial startup - from 1 to 36."));
    months2loadSpinBoxInfo->setFont(ff);
    months2loadSpinBox = new QSpinBox;
    months2loadSpinBox->setRange(1,36);
    months2loadSpinBox->setMaximumWidth(60);
    months2loadSpinBox->setValue(mainwindow->nestOptions.months2load);
    QLabel *months2loadSpinBoxInfoRestart = new QLabel(tr("Restart is required for changes to take effect."));
    months2loadSpinBoxInfoRestart->setFont(ii);

    QVBoxLayout *vlayout4 = new QVBoxLayout;
    vlayout4->addWidget(allowDeleteRecordCheckBox);
    vlayout4->addWidget(deleteRecordInfo);
    vlayout4->addSpacing(40);
    vlayout4->addWidget(months2loadSpinBoxInfo);
    vlayout4->addWidget(months2loadSpinBox);
    vlayout4->addWidget(months2loadSpinBoxInfoRestart);
    vlayout4->addStretch();
    tab4->setLayout(vlayout4);
    tab4->adjustSize();


    // =============== Commons ====================

    cancelButton = new QPushButton;
    cancelButton->setText(tr("Cancel"));
    cancelButton->setIcon(mainwindow->style()->standardIcon(QStyle::SP_DialogCancelButton));
    saveButton = new QPushButton;
    saveButton->setText(tr("Save"));
    saveButton->setIcon(mainwindow->style()->standardIcon(QStyle::SP_DialogSaveButton));

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(saveButton);
    //buttonsLayout->addStretch();
    buttonsLayout->addWidget(cancelButton);

    tabholder->addTab(tab1, "&Refresh settings");
    tabholder->addTab(tab2, "E&xternal Program");
    tabholder->addTab(tab3, "&EDF export");
    tabholder->addTab(tab4, "Other");

    QHBoxLayout *horLayout = new QHBoxLayout;
    horLayout->addStretch(1000);
    horLayout->addLayout(buttonsLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabholder);
    mainLayout->addSpacing(20);
    mainLayout->addLayout(horLayout);

    optionsdialog->setMinimumSize(900, 600);

    optionsdialog->setLayout(mainLayout);


    //======== SLOTS ========
    // REFRESHING SETTINGS
    connect(buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(modeButtonClicked(int)));
    connect(hourDial, SIGNAL(valueChanged(int)), this, SLOT(hourDialValueChanged(int)));
    connect(usePeriodic, SIGNAL(toggled(bool)), this, SLOT(enablePeriodicRefreshing(bool)));
    connect(useWorkingHours, SIGNAL(toggled(bool)), this, SLOT(enableWorkingHoursOnly(bool)));
    connect(loadStaticOnRefresh, SIGNAL(toggled(bool)), this, SLOT(enableLoadStaticOnRefresh(bool)));
    // EXTERNAL PROGRAMS
    connect(AddBrainlabReaderButton, SIGNAL(clicked()), this,SLOT(add_brainlab_reader()));
    connect(AddBrainlabControlButton, SIGNAL(clicked()), this,SLOT(add_brainlab_control()));
    connect(AddHarmonieReaderButton, SIGNAL(clicked()), this,SLOT(add_harmonie_reader()));
    connect(AddDicomReaderButton, SIGNAL(clicked()), this,SLOT(add_dicom_reader()));
    connect(allowDicomCheckBox, SIGNAL(toggled(bool)), this, SLOT(enableDicom(bool)));
    // EDF export
    connect(changeExportProgramButton, SIGNAL(clicked()), this,SLOT(add_exporter()));
    connect(changeExportPathButton, SIGNAL(clicked()), this,SLOT(add_path2export()));
    connect(anonymizeCheckBox, SIGNAL(toggled(bool)), this, SLOT(enableAnonymize(bool)));
    connect(shortenCheckBox, SIGNAL(toggled(bool)), this, SLOT(enableShorten(bool)));
    connect(allowExportCheckBox, SIGNAL(toggled(bool)), this, SLOT(enableExport(bool)));
    connect(systemEventsCheckBox, SIGNAL(toggled(bool)), this, SLOT(enableSystemEventsExport(bool)));
    connect(enableDebugModeCheckBox, SIGNAL(toggled(bool)), this, SLOT(enableDebugMode(bool)));
    // OTHER
    connect(allowDeleteRecordCheckBox, SIGNAL(toggled(bool)), this, SLOT(enableDelete(bool)));
    // SAVE or CANCEL
    connect(cancelButton,   SIGNAL(clicked()), optionsdialog, SLOT(close()));
    connect(saveButton,   SIGNAL(clicked()), this, SLOT(saveAndClose()));

    // LETS DO IT!
    optionsdialog->exec();

}

void OptionsDialog::loadSettingsFromMainWindow(){

    un_options = mainwindow->nestOptions;
}


void OptionsDialog::modeButtonClicked(int value){
    un_options.periodicRefreshMode = value;
}

void OptionsDialog::hourDialValueChanged(int value){

    //qDebug() << "value " << value;

    if (value > 30){
        un_options.refreshingPeriod = value -30;
    }
    else{
        un_options.refreshingPeriod = value + 30;
    }

    //qDebug() << "refreshing period " << refreshingPeriod;
    timeLabel->setText(QString("Refresh every %1 minutes").arg(un_options.refreshingPeriod));
    //timeLabel->update();

}

void OptionsDialog::enablePeriodicRefreshing(bool checked){
    hourDial->setEnabled(checked);
    useWorkingHours->setEnabled(checked);
    timeLabel->setEnabled(checked);
    un_options.periodicRefreshingEnabled = checked;
}

void OptionsDialog::enableWorkingHoursOnly(bool checked){
    un_options.refreshWorkingHoursOnly = checked;
    qDebug() << checked;
}

void OptionsDialog::enableLoadStaticOnRefresh(bool checked){
    un_options.refreshLoadStatic = checked;
    qDebug() << checked;
}

void OptionsDialog::add_brainlab_reader(){
    add_program("brainlabReader");
}

void OptionsDialog::add_brainlab_control(){
    add_program("brainlabControl");
}

void OptionsDialog::add_harmonie_reader(){
    add_program("harmonie");
}

void OptionsDialog::add_dicom_reader(){
    add_program("dicom");
}

void OptionsDialog::add_program(QString program){

    QString temp = QFileDialog::getOpenFileName(0, tr("Choose external program"), mainwindow->nestOptions.defaultReaderFolder, tr("*.exe"));

    if(temp.isEmpty()){
        return;
    }

    if(program == "brainlabReader"){
        brainlabReaderEdit->setText(temp);
    }

    if(program == "brainlabControl"){
        brainlabControlEdit->setText(temp);
    }

    if(program == "harmonie"){
        harmonieReaderEdit->setText(temp);
    }

    if(program == "dicom"){
        dicomReaderEdit->setText(temp);
    }
}

void OptionsDialog::enableDicom(bool checked){
    un_options.dicomReaderEnable = checked;
    dicomReaderEdit->setEnabled(checked);
    AddDicomReaderButton->setEnabled(checked);
}

void OptionsDialog::enableExport(bool checked){
    un_options.exportAllow = checked;
    exportEdit->setEnabled(checked);
    changeExportProgramButton->setEnabled(checked);
    exportPathEdit->setEnabled(checked);
    changeExportPathButton->setEnabled(checked);
    anonymizeCheckBox->setEnabled(checked);
    shortenCheckBox->setEnabled(checked);
    systemEventsCheckBox->setEnabled(checked);
    enableDebugModeCheckBox->setEnabled(checked);
}


void OptionsDialog::add_exporter(){
    QString temp = QFileDialog::getOpenFileName(0, tr("Choose EDF exporter"), "", tr("Cuculus(*.exe)"));

    if(temp.isEmpty()){
        return;
    }else{
        exportEdit->setText(temp);
    }
}

void OptionsDialog::add_path2export(){
    QString temp = QFileDialog::getExistingDirectory(0, tr("Choose directory for export"), "");

    if(temp.isEmpty()){
        return;
    }else{
        exportPathEdit->setText(temp);
    }

}

void OptionsDialog::enableAnonymize(bool checked){
    un_options.exportAnonymize= checked;
}

void OptionsDialog::enableShorten(bool checked){
    un_options.exportShortenLabels= checked;
}

void OptionsDialog::enableSystemEventsExport(bool checked){
    un_options.exportSystemEvents = checked;
}

void OptionsDialog::enableDebugMode(bool checked){
    un_options.exportEnableDebug = checked;
}

void OptionsDialog::enableDelete(bool checked){
    un_options.recordDeleteAllow = checked;
}

void OptionsDialog::saveAndClose(){

    // REFRESHING SETTINGS
    // refreshing period should not be 0
    if(un_options.refreshingPeriod == 0){
        un_options.refreshingPeriod = 1;
    }

    //    // EXTERNAL PROGRAMS
    un_options.brainlabReader = brainlabReaderEdit->text();
    un_options.brainlabControl = brainlabControlEdit->text();
    un_options.harmonieReader = harmonieReaderEdit->text();
    un_options.dicomReaderPath = dicomReaderEdit->text();
    //    //EDF EXPORT
    un_options.exportProgram = exportEdit->text();
    un_options.exportPath =  exportPathEdit->text();

    // MONTHS to load
    //qDebug() << months2loadSpinBox->value();
    un_options.months2load = months2loadSpinBox->value();


    //qDebug() << "Save and Close";
    //qDebug() << "Refresh mode: " << un_options.periodicRefreshMode;
    //qDebug() << "Load static on refresh: " << un_options.refreshLoadStatic;

    mainwindow->nestOptions = un_options;

    optionsdialog->close();
}
