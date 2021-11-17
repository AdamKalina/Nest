#ifndef REFRESHSETTINGS_H
#define REFRESHSETTINGS_H
#include <QDial>
#include <QCheckBox>
#include <QButtonGroup>
#include "mainwindow.h"

class MainWindow;

class refreshSettings : public QObject
{

    Q_OBJECT

public:
    // std
    int refreshingPeriod; // in min
    int RefreshModeId;
    bool periodicRefreshingEnabled;
    bool workingHoursOnly;

    // Qt
    MainWindow *mainwindow;
    QDialog      *edit_refresh_settings;
    QLabel *dialLabel, *timeLabel;
    QCheckBox *usePeriodic, *useWorkingHours;
    QDial *hourDial;
    QPushButton *cancelButton, *saveButton;

    //functions
    refreshSettings(QWidget *parent);
    void loadSettingsFromMainWindow();


public slots:
    void hourDialValueChanged(int value);
    void enablePeriodicRefreshing(bool state);
    void enableWorkingHoursOnly(bool state);
    void saveAndClose();
    void modeButtonClicked(int value);
};

#endif // REFRESHSETTINGS_H

