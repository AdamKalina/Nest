#ifndef TABOPTIONS_H
#define TABOPTIONS_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QTabWidget>
#include <QGroupBox>
#include <QButtonGroup>
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
    *AddReaderButton,
    *AddControlButton,
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

    QLabel *dialLabel, *timeLabel, *exportLabel, *exportPathLabel;
    QCheckBox *usePeriodic,
    *useWorkingHours,
    *anonymizeCheckBox,
    *shortenCheckBox,
    *systemEventsCheckBox,
    *enableDebugModeCheckBox;
    QDial *hourDial;

    QLineEdit *readerEdit,
    *controlEdit,
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
    void add_reader();
    void add_control();
    void add_exporter();
    void add_path2export();
    void enableAnonymize(bool checked);
    void enableShorten(bool checked);
    void enableExport(bool checked);
};


#endif // TABOPTIONS_H
