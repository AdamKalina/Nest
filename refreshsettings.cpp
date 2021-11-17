#include "refreshsettings.h"

refreshSettings::refreshSettings(QWidget *w_parent)
{
    mainwindow = (MainWindow *)w_parent;

    loadSettingsFromMainWindow();

    QFont b( "Arial", 10, QFont::Bold);
    QFont ff( "Arial", 10);

    edit_refresh_settings = new QDialog;
    edit_refresh_settings->setMinimumSize(800, 265);
    edit_refresh_settings->setWindowTitle("Edit resfresh settings");
    edit_refresh_settings->setModal(true);
    edit_refresh_settings->setAttribute(Qt::WA_DeleteOnClose, true);


    //======== FILES SETTINGS ========
    QLabel *filesLabel = new QLabel(tr("Files setting - not yet ready"));
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

    buttonGroup->button(RefreshModeId)->setChecked(1);

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

    if(refreshingPeriod > 30){
        dialValue = refreshingPeriod - 30;
    }else{
        dialValue = refreshingPeriod + 30;
    }

    hourDial->setValue(dialValue);
    hourDial->setWrapping(1);
    hourDial->setNotchesVisible(1);
    hourDial->setEnabled(periodicRefreshingEnabled);

    timeLabel = new QLabel(QString("Refresh automatically every %1 minutes").arg(refreshingPeriod)); // construct like this so it can be updated
    timeLabel->setFont(ff);
    timeLabel->setEnabled(periodicRefreshingEnabled);

    QCheckBox *usePeriodic = new QCheckBox("Use periodic refreshing");
    usePeriodic->setFont(ff);
    usePeriodic->setChecked(periodicRefreshingEnabled);

    useWorkingHours = new QCheckBox("Use in working hours only - not yet ready");
    useWorkingHours->setFont(ff);
    useWorkingHours->setEnabled(periodicRefreshingEnabled);
    useWorkingHours->setChecked(workingHoursOnly);

    // ======== BUTTONS ========

    cancelButton = new QPushButton;
    cancelButton->setText(tr("Cancel"));
    saveButton = new QPushButton;
    saveButton->setText(tr("Save"));


    // ======== LAYOUT ORGANIZATION ========
    QVBoxLayout *bottomLeft = new QVBoxLayout;
   // bottomLeft->addStretch();
    //bottomLeft->setSpacing(4);
    bottomLeft->addWidget(usePeriodic);
    bottomLeft->addWidget(useWorkingHours);
    bottomLeft->addStretch();

    QVBoxLayout *bottomRight = new QVBoxLayout;
    bottomRight->addWidget(hourDial);
    bottomRight->addWidget(timeLabel, Qt::AlignCenter);

    QHBoxLayout *bottom = new QHBoxLayout;
    bottom->addLayout(bottomLeft);
    bottom->addLayout(bottomRight);

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(saveButton);
    //buttonsLayout->addStretch();
    buttonsLayout->addWidget(cancelButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(filesLabel);
    mainLayout->addWidget(refreshAll);
    mainLayout->addWidget(refreshYear);
    mainLayout->addWidget(refreshMonth);
    mainLayout->addWidget(refreshWeek);
    mainLayout->addWidget(refreshToday);
    mainLayout->addSpacing(20);
    //mainLayout->addSpacerItem()
    mainLayout->addWidget(dialLabel);
    mainLayout->addLayout(bottom);
    mainLayout->addSpacing(20);
    mainLayout->addLayout(buttonsLayout);
    edit_refresh_settings->setLayout(mainLayout);

    // ======== SLOTS ========

    connect(buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(modeButtonClicked(int)));
    connect(hourDial, SIGNAL(valueChanged(int)), this, SLOT(hourDialValueChanged(int)));
    connect(usePeriodic, SIGNAL(toggled(bool)), this, SLOT(enablePeriodicRefreshing(bool)));
    connect(useWorkingHours, SIGNAL(toggled(bool)), this, SLOT(enableWorkingHoursOnly(bool)));
    connect(cancelButton,   SIGNAL(clicked()), edit_refresh_settings, SLOT(close()));
    connect(saveButton,   SIGNAL(clicked()), this, SLOT(saveAndClose()));

    // ======== RUN THIS THING ========
    edit_refresh_settings->exec();
}

void refreshSettings::loadSettingsFromMainWindow(){
    RefreshModeId = mainwindow->periodicRefreshMode;
    refreshingPeriod = mainwindow->refreshingPeriod;
    periodicRefreshingEnabled = mainwindow->periodicRefreshingEnabled;
    workingHoursOnly = mainwindow->workingHoursOnly;
}

void refreshSettings::modeButtonClicked(int value){
    RefreshModeId = value;
}

void refreshSettings::hourDialValueChanged(int value){

    //qDebug() << value;

    if (value > 30){
        refreshingPeriod = value -30;
    }
    else{
        refreshingPeriod = value + 30;
    }
    timeLabel->setText(QString("Refresh every %1 minutes").arg(refreshingPeriod));
    //timeLabel->update();

}

void refreshSettings::enablePeriodicRefreshing(bool checked){
    hourDial->setEnabled(checked);
    useWorkingHours->setEnabled(checked);
    timeLabel->setEnabled(checked);
    periodicRefreshingEnabled = checked;
}

void refreshSettings::enableWorkingHoursOnly(bool checked){
    workingHoursOnly = checked;
    qDebug() << checked;
}

void refreshSettings::saveAndClose(){

    //qDebug() << "period" << refreshingPeriod;
    //qDebug() << "usePeriodic" << periodicRefreshingEnabled;

    mainwindow->periodicRefreshMode = RefreshModeId;
    mainwindow->refreshingPeriod = refreshingPeriod;
    mainwindow->periodicRefreshingEnabled = periodicRefreshingEnabled;
    mainwindow->workingHoursOnly = workingHoursOnly;
    mainwindow->refreshQTimer();
    mainwindow->workingHoursQTimer();

    //qDebug() << "Save and Close";
    qDebug() << "Refresh mode: " << RefreshModeId;
    edit_refresh_settings->close();
}
