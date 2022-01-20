#include "refreshsettings.h"

refreshSettings::refreshSettings(QWidget *w_parent)
{
    mainwindow = (MainWindow *)w_parent;

    loadSettingsFromMainWindow();

    QFont b("Arial", 10, QFont::Bold);
    QFont ff("Arial", 10);
    QFont ii("Arial", 10, QFont::StyleItalic);
    ii.setItalic(true);

    edit_refresh_settings = new QDialog;
    edit_refresh_settings->setMinimumSize(800, 265);
    edit_refresh_settings->setWindowTitle("Edit resfresh settings");
    edit_refresh_settings->setModal(true);
    edit_refresh_settings->setAttribute(Qt::WA_DeleteOnClose, true);


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

    buttonGroup->button(RefreshModeId)->setChecked(1);

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
    QLabel *usePeriodicInfo = new QLabel(tr("If checked it will perform refresh of dynamic folders in interval set on the dial."));
    usePeriodicInfo->setFont(ii);

    useWorkingHours = new QCheckBox("Use in working hours only");
    useWorkingHours->setFont(ff);
    useWorkingHours->setEnabled(periodicRefreshingEnabled);
    useWorkingHours->setChecked(workingHoursOnly);
    QLabel *useWorkingHoursInfo = new QLabel(tr("If it will perform periodic refresh in working hours only - set from 7 AM to 17 PM"));
    useWorkingHoursInfo->setFont(ii);

    // ======== REFRESH STATIC ========

    QLabel *staticLabel = new QLabel(tr("Static folders setting"));
    staticLabel->setFont(b);

    QCheckBox *loadStaticOnRefresh = new QCheckBox("Load static data on refresh");
    loadStaticOnRefresh->setFont(ff);
    loadStaticOnRefresh->setChecked(loadStaticOnRefreshEnabled);
    QLabel *staticLabelInfo = new QLabel(tr("If checked it will load data from refreshed static folders into model.\nIf unchecked it will only refresh database entries."));
    staticLabelInfo->setFont(ii);


    // ======== BUTTONS ========

    cancelButton = new QPushButton;
    cancelButton->setText(tr("Cancel"));
    cancelButton->setIcon(mainwindow->style()->standardIcon(QStyle::SP_DialogCancelButton));
    saveButton = new QPushButton;
    saveButton->setText(tr("Save"));
    saveButton->setIcon(mainwindow->style()->standardIcon(QStyle::SP_DialogSaveButton));


    // ======== LAYOUT ORGANIZATION ========
    QVBoxLayout *bottomLeft = new QVBoxLayout;
   // bottomLeft->addStretch();
    //bottomLeft->setSpacing(4);
    bottomLeft->addWidget(usePeriodic);
    bottomLeft->addWidget(usePeriodicInfo);
    bottomLeft->addSpacing(10);
    bottomLeft->addWidget(useWorkingHours);
    bottomLeft->addWidget(useWorkingHoursInfo);
    bottomLeft->addSpacing(10);
    bottomLeft->addWidget(staticLabel);
    bottomLeft->addWidget(loadStaticOnRefresh);
    bottomLeft->addWidget(staticLabelInfo);
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
    mainLayout->addWidget(filesInfo);
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
    connect(loadStaticOnRefresh, SIGNAL(toggled(bool)), this, SLOT(enableLoadStaticOnRefresh(bool)));
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
    loadStaticOnRefreshEnabled = mainwindow->loadStaticOnRefreshEnabled;
}

void refreshSettings::modeButtonClicked(int value){
    RefreshModeId = value;
}

void refreshSettings::hourDialValueChanged(int value){

    //qDebug() << "value " << value;

    if (value > 30){
        refreshingPeriod = value -30;
    }
    else{
        refreshingPeriod = value + 30;
    }

    //qDebug() << "refreshing period " << refreshingPeriod;
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

void refreshSettings::enableLoadStaticOnRefresh(bool checked){
    loadStaticOnRefreshEnabled = checked;
    qDebug() << checked;
}

void refreshSettings::saveAndClose(){

    //qDebug() << "period" << refreshingPeriod;
    //qDebug() << "usePeriodic" << periodicRefreshingEnabled;

    mainwindow->periodicRefreshMode = RefreshModeId;

    // refreshing period should not be 0
    if(refreshingPeriod == 0){
        refreshingPeriod = 1;
    }

    mainwindow->refreshingPeriod = refreshingPeriod;
    mainwindow->periodicRefreshingEnabled = periodicRefreshingEnabled;
    mainwindow->workingHoursOnly = workingHoursOnly;
    mainwindow->loadStaticOnRefreshEnabled = loadStaticOnRefreshEnabled;
    mainwindow->refreshQTimer();
    mainwindow->workingHoursQTimer();

    //qDebug() << "Save and Close";
    qDebug() << "Refresh mode: " << RefreshModeId;
    qDebug() << "Load static on refresh: " << loadStaticOnRefreshEnabled;
    edit_refresh_settings->close();
}
