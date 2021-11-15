#include "externalprogramlist.h"

externalprogramlist::externalprogramlist(QWidget *w_parent)
{
    mainwindow = (MainWindow *)w_parent;

    QFont f( "Arial", 10, QFont::Bold);

    edit_program_dialog = new QDialog;
    edit_program_dialog->setMinimumSize(800, 265);
    edit_program_dialog->setWindowTitle("Edit used programs");
    edit_program_dialog->setModal(true);
    edit_program_dialog->setAttribute(Qt::WA_DeleteOnClose, true);

    QLabel *scanLabel = new QLabel(tr("EEG reader"));
    scanLabel->setFont(f);

    QLineEdit *scanEdit = new QLineEdit(mainwindow->externalProgram1);

    QLabel *controlLabel = new QLabel(tr("EEG reader for recorded files (control)"));
    controlLabel->setFont(f);

    QLineEdit *controlEdit = new QLineEdit(mainwindow->externalProgram2);


    CloseButton = new QPushButton;
    CloseButton->setText("Close");

    QHBoxLayout *hlayout1 = new QHBoxLayout;
    hlayout1->addStretch(1200);
    hlayout1->addWidget(CloseButton);

    QVBoxLayout *vlayout1 = new QVBoxLayout;
    vlayout1->addWidget(scanLabel);
    vlayout1->addWidget(scanEdit);
    vlayout1->addSpacing(10);
    vlayout1->addWidget(controlLabel);
    vlayout1->addWidget(controlEdit);
    vlayout1->addLayout(hlayout1);

    edit_program_dialog->setLayout(vlayout1);
    edit_program_dialog->adjustSize();

    connect(CloseButton,   SIGNAL(clicked()), edit_program_dialog, SLOT(close()));
    //connect(program_list, SIGNAL(rowClicked(QListWidgetItem*)), this,                          SLOT(rowClicked(QListWidgetItem *)));

    edit_program_dialog->exec();


}
