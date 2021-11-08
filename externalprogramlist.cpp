#include "externalprogramlist.h"

externalprogramlist::externalprogramlist(QWidget *w_parent)
{
    mainwindow = (MainWindow *)w_parent;

    edit_program_dialog = new QDialog;
    edit_program_dialog->setMinimumSize(800, 265);
    edit_program_dialog->setWindowTitle("Edit used programs");
    edit_program_dialog->setModal(true);
    edit_program_dialog->setAttribute(Qt::WA_DeleteOnClose, true);

    program_list = new QListWidget;
    program_list->setSelectionBehavior(QAbstractItemView::SelectRows);
    program_list->setSelectionMode(QAbstractItemView::SingleSelection);
    program_list->setSpacing(1);

    new QListWidgetItem(mainwindow->externalProgram1, program_list);
    new QListWidgetItem(mainwindow->externalProgram2, program_list);

    qDebug() << mainwindow->externalProgram1;


    CloseButton = new QPushButton;
    CloseButton->setText("Close");

    QHBoxLayout *hlayout1 = new QHBoxLayout;
    hlayout1->addStretch(1000);
    hlayout1->addWidget(CloseButton);

    QVBoxLayout *vlayout1 = new QVBoxLayout;
    vlayout1->addWidget(program_list, 1000);
    vlayout1->addSpacing(20);
    vlayout1->addLayout(hlayout1);

    edit_program_dialog->setLayout(vlayout1);

    connect(CloseButton,   SIGNAL(clicked()), edit_program_dialog, SLOT(close()));
    //connect(program_list, SIGNAL(rowClicked(QListWidgetItem*)), this,                          SLOT(rowClicked(QListWidgetItem *)));

    edit_program_dialog->exec();


}
