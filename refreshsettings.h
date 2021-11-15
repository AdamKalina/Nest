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
    refreshSettings(QWidget *parent);
    MainWindow *mainwindow;
    QDialog      *edit_refresh_settings;
    QLabel *dialLabel, *timeLabel;
    QCheckBox *usePeriodic, *useWorkingHours;
    QDial *hourDial;
    QPushButton *cancelButton, *saveButton;
    int refreshingPeriod; // in ms
    bool periodicRefreshingEnabled;

public slots:
    void hourDialValueChanged(int value);
    void enablePeriodicRefreshing(bool state);
    void saveAndClose();
};

#endif // REFRESHSETTINGS_H

