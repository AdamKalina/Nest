#include "nest_options.h"

nest_options_struct nest_options::readSettings()
{
    nest_options_struct n_opt;
    qDebug() << "nest_options::readSettings()";
    QSettings settings("settings.ini",QSettings::IniFormat);
    n_opt.brainlabReader = settings.value("brainlab_reader").toString();
    n_opt.brainlabControl = settings.value("brainlab_control").toString();
    n_opt.nicoletReader = settings.value("nicolet_reader").toString();
    n_opt.harmonieReader = settings.value("harmonie_reader").toString();
    n_opt.defaultDataFolder = settings.value("defaultDataFolder").toString();
    n_opt.defaultReaderFolder = settings.value("defaultReaderFolder").toString();
    n_opt.refreshingPeriod = settings.value("refreshing_period").toInt();
    n_opt.periodicRefreshingEnabled = settings.value("periodic_refreshing_enabled").toBool();
    n_opt.refreshLoadStatic = settings.value("load_static_on_refresh_enabled").toBool();
    n_opt.refreshWorkingHoursOnly = settings.value("periodic_refreshing_in_working_hours_only").toBool();
    n_opt.periodicRefreshMode = settings.value("periodic_refresh_mode").toInt();
    n_opt.boldParent = settings.value("bold_parent").toBool();
    n_opt.exportAnonymize = settings.value("anonymize_export").toBool();
    n_opt.exportProgram = settings.value("export_program").toString();
    n_opt.exportPath = settings.value("export_path").toString();
    n_opt.exportAllow = settings.value("allow_export").toBool();
    n_opt.exportShortenLabels = settings.value("shorten_export").toBool();
    n_opt.exportSystemEvents = settings.value("export_system_events").toBool();
    n_opt.recordDeleteAllow = settings.value("enable_delete_records").toBool();
    n_opt.exportEnableDebug = settings.value("enable_export_debug_mode").toBool();
    n_opt.months2load = settings.value("months_to_load").toInt();
    n_opt.patients2load_startup = settings.value("patients_to_load_start").toInt();
    n_opt.patients2load_add = settings.value("patients_to_load_add").toInt();
    n_opt.readNicOneDb = settings.value("read_NicoletOne_db").toBool();
    n_opt.NicOneDbPath = settings.value("NicoletOne_db_path").toString();


    //// Signal folders ////
    std::vector<std::string> systems = {"Brainlab", "Harmonie", "Nicolet"};
    std::vector<signal_dirs*> dirs = {&n_opt.Brainlab_dirs,&n_opt.Harmonie_dirs,&n_opt.Nicolet_dirs};

    for(unsigned int s = 0; s < systems.size(); s++){
        settings.beginGroup(QString::fromStdString(systems[s]));

        // load array of static folders
        int size = settings.beginReadArray("static_dirs");
        for (int i = 0; i < size; ++i) {
            settings.setArrayIndex(i);
            QString new_stat_dir = settings.value("path").toString();
            dirs[s]->static_dirs.append(new_stat_dir);
        }
        settings.endArray();

        // load array of dynamic folders
        size = settings.beginReadArray("dynamic_dirs");
        for (int i = 0; i < size; ++i) {
            settings.setArrayIndex(i);
            QString new_dyn_dir = settings.value("path").toString();
            dirs[s]->dynamic_dirs.append(new_dyn_dir);
        }
        settings.endArray();

        settings.endGroup();
    }

    // load array of used drives (not used right now)
    int size = settings.beginReadArray("used_drives");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QString usedDrive = settings.value("path").toString();
        n_opt.usedDrives.append(usedDrive);
    }
    settings.endArray();

    // load array of batch files - paired with used drives
    size = settings.beginReadArray("batch_files");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QString batchFile = settings.value("path").toString();
        n_opt.batchFiles.append(batchFile);
    }
    settings.endArray();

    return n_opt;
}

void nest_options::writeSettings(nest_options_struct n_opt){
    QSettings settings("settings.ini",QSettings::IniFormat);
    settings.setValue("brainlab_reader", n_opt.brainlabReader);
    settings.setValue("brainlab_control", n_opt.brainlabControl);
    settings.setValue("nicolet_reader", n_opt.nicoletReader);
    settings.setValue("harmonie_reader", n_opt.harmonieReader);
    settings.setValue("defaultDataFolder","D:/Dropbox/Scripts/Cpp/");
    settings.setValue("defaultReaderFolder","D:/Dropbox/Scripts/Cpp/EEGLE/build-EEGle-Desktop_Qt_5_15_2_MinGW_64_bit-Release/");
    settings.setValue("refreshing_period",n_opt.refreshingPeriod);
    settings.setValue("periodic_refreshing_enabled", n_opt.periodicRefreshingEnabled);
    settings.setValue("load_static_on_refresh_enabled",n_opt.refreshLoadStatic);
    settings.setValue("periodic_refreshing_in_working_hours_only",n_opt.refreshWorkingHoursOnly);
    settings.setValue("periodic_refresh_mode",n_opt.periodicRefreshMode);
    settings.setValue("bold_parent",n_opt.boldParent);
    settings.setValue("allow_export", n_opt.exportAllow);
    settings.setValue("anonymize_export",n_opt.exportAnonymize);
    settings.setValue("export_program",n_opt.exportProgram);
    settings.setValue("export_path",n_opt.exportPath);
    settings.setValue("shorten_export",n_opt.exportShortenLabels);
    settings.setValue("export_system_events",n_opt.exportSystemEvents);
    settings.setValue("enable_delete_records",n_opt.recordDeleteAllow);
    settings.setValue("enable_export_debug_mode",n_opt.exportEnableDebug);
    settings.setValue("months_to_load",n_opt.months2load);
    settings.setValue("patients_to_load_start",n_opt.patients2load_startup);
    settings.setValue("patients_to_load_add",n_opt.patients2load_add);
    settings.setValue("read_NicoletOne_db",n_opt.readNicOneDb);
    settings.setValue("NicoletOne_db_path",n_opt.NicOneDbPath);

    //// Signal folders ////
    std::vector<std::string> systems = {"Brainlab", "Harmonie", "Nicolet"};
    std::vector<signal_dirs*> dirs = {&n_opt.Brainlab_dirs,&n_opt.Harmonie_dirs,&n_opt.Nicolet_dirs};

    for(unsigned int s = 0; s < systems.size(); s++){
        settings.beginGroup(QString::fromStdString(systems[s]));

        dirs[s]->static_dirs.sort(); // sort the folder alphabetically
        settings.beginWriteArray("static_dirs");
        for (int i = 0; i < dirs[s]->static_dirs.size(); ++i) {
            settings.setArrayIndex(i);
            settings.setValue("path", dirs[s]->static_dirs.at(i));
        }
        settings.endArray();

        dirs[s]->dynamic_dirs.sort();
        settings.beginWriteArray("dynamic_dirs");
        for (int i = 0; i < dirs[s]->dynamic_dirs.size(); ++i) {
            settings.setArrayIndex(i);
            settings.setValue("path", dirs[s]->dynamic_dirs.at(i));
        }
        settings.endArray();

        settings.endGroup();
    }
}
