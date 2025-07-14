#ifndef TABOPTIONS_H
#define TABOPTIONS_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QTabWidget>
#include <QGroupBox>
#include <QButtonGroup>
#include <QSpinBox>
#include "mainwindow.h"

class MainWindow;

class n_options;

class OptionsDialog : public QObject
{
    Q_OBJECT

public:
    OptionsDialog(QWidget *parent=nullptr);

    MainWindow *mainwindow;

    n_options un_options;

private:

    QDialog        *optionsdialog;
    QPushButton    *cancelButton,
    *saveButton,
    *AddBrainlabReaderButton,
    *AddBrainlabControlButton,
    *AddNicoletReaderButton,
    *AddHarmonieReaderButton,
    *AddDicomReaderButton,
    *changeExportPathButton,
    *changeExportProgramButton;

    QTabWidget     *tabholder;

    QWidget        *tab1,
    *tab2,
    *tab3,
    *tab4,
    *tab5,
    *tab6,
    *tab7;

    QLabel *dialLabel, *timeLabel, *exportLabel, *exportPathLabel, *patients2loadAddSpinBoxInfo;
    QCheckBox *usePeriodic,
    *useWorkingHours,
    *anonymizeCheckBox,
    *shortenCheckBox,
    *systemEventsCheckBox,
    *enableDebugModeCheckBox;
    QDial *hourDial;

    QSpinBox *months2loadSpinBox, *patients2loadStartSpinBox, *patients2loadAddSpinBox;

    QLineEdit *brainlabReaderEdit,
    *brainlabControlEdit,
    *nicoletReaderEdit,
    *harmonieReaderEdit,
    *dicomReaderEdit,
    *exportEdit,
    *exportPathEdit;

    void add_program(QString program);
    void loadSettingsFromMainWindow();

public slots:
    void hourDialValueChanged(int value);
    void enablePeriodicRefreshing(bool checked);
    void enableWorkingHoursOnly(bool checked);
    void enableLoadStaticOnRefresh(bool checked);
    void saveAndClose();
    void modeButtonClicked(int value);
    void add_brainlab_reader();
    //void add_brainlab_control();
    void add_harmonie_reader();
    void add_nicolet_reader();
    void add_dicom_reader();
    void add_exporter();
    void add_path2export();
    void enableAnonymize(bool checked);
    void enableShorten(bool checked);
    void enableExport(bool checked);
    void enableSystemEventsExport(bool checked);
    void enableDebugMode(bool checked);
    void enableDelete(bool checked);
    void enableDicom(bool checked);
};


#endif // TABOPTIONS_H
