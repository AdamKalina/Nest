#ifndef NEST_OPTIONS_H
#define NEST_OPTIONS_H

#include <QString>
#include <QStringList>
#include <QSettings>
#include <QDebug>

struct signal_dirs{
    QStringList dynamic_dirs;
    QStringList static_dirs;
};

struct nest_options_struct{
    // general
    int months2load = 24;
    int patients2load_startup = 100;
    int patients2load_add = 10;
    bool boldParent = false;

    // refreshing settings
    int refreshingPeriod = 15; // in minutes
    bool periodicRefreshingEnabled = false;
    int periodicRefreshMode = 0;
    bool refreshWorkingHoursOnly = false;
    bool refreshLoadStatic = true;

    // EEG reader
    QString brainlabReader; // for regular files, scan.exe. in XP
    QString brainlabControl; // for files being recorded - control.exe in XP
    QString harmonieReader;
    QString nicoletReader;
    QString defaultReaderFolder;

    // EDF export
    QString exportProgram;
    QString exportPath;
    bool exportAnonymize = false;
    bool exportAllow = true;
    bool exportShortenLabels = false;
    bool exportSystemEvents = false;
    bool exportEnableDebug = false;

    // EEG folders
    QString defaultDataFolder;
    signal_dirs Brainlab_dirs;
    signal_dirs Harmonie_dirs;
    signal_dirs Nicolet_dirs;
    QStringList usedDrives;
    QStringList batchFiles;

    // user editing of db
    bool recordDeleteAllow = false;
    // reading data from NicoletOne db
    bool readNicOneDb = true;
    QString NicOneDbPath;
};

class nest_options{

public:
    // functions
    void writeSettings(nest_options_struct n_opt);
    nest_options_struct readSettings();
};

#endif // NEST_OPTIONS_H
