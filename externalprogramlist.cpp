#include "externalprogramlist.h"

externalprogramlist::externalprogramlist(QWidget *w_parent)
{
    mainwindow = (MainWindow *)w_parent;

    QFont f("Arial", 10, QFont::Bold);

    edit_program_dialog = new QDialog;
    edit_program_dialog->setMinimumSize(800, 265);
    edit_program_dialog->setWindowTitle("Edit used programs");
    edit_program_dialog->setModal(true);
    edit_program_dialog->setAttribute(Qt::WA_DeleteOnClose, true);

    QLabel *readerLabel = new QLabel(tr("EEG reader"));
    readerLabel->setFont(f);
    readerEdit = new QLineEdit(mainwindow->externalProgram1);

    QLabel *controlLabel = new QLabel(tr("EEG reader for recorded files (control)"));
    controlLabel->setFont(f);
    controlEdit = new QLineEdit(mainwindow->externalProgram2);

    cancelButton = new QPushButton;
    cancelButton->setText(tr("Cancel"));
    cancelButton->setIcon(mainwindow->style()->standardIcon(QStyle::SP_DialogCancelButton));

    saveButton = new QPushButton;
    saveButton->setText(tr("Save"));
    saveButton->setIcon(mainwindow->style()->standardIcon(QStyle::SP_DialogSaveButton));

    AddReaderButton = new QPushButton;
    AddReaderButton->setText(tr("Add reader"));
    AddReaderButton->setIcon(mainwindow->style()->standardIcon(QStyle::SP_FileDialogNewFolder));

    AddControlButton = new QPushButton;
    AddControlButton->setText(tr("Add control"));
    AddControlButton->setIcon(mainwindow->style()->standardIcon(QStyle::SP_FileDialogNewFolder));

    // vlayout1
    //  ---------------------------------------------------
    // |scanLabel                                          |
    // |---------------------------------------------------|
    // ||-------------------------------------------------||
    // ||hlayout_reader                                   ||
    // ||readerEdit                        AddReaderButton||
    // ||-------------------------------------------------||
    // |---------------------------------------------------|
    // |spacing 20 px                                      |
    // |---------------------------------------------------|
    // |controlLabel                                       |
    // |---------------------------------------------------|
    // ||-------------------------------------------------||
    // ||hlayout_control                                  ||
    // ||controlEdit                      AddControlButton||
    // ||-------------------------------------------------||
    // |---------------------------------------------------|
    // |spacing 20 px                                      |
    // |---------------------------------------------------|
    // |hlayout1                                           |
    // |                                 Apply Close       |
    //  ---------------------------------------------------

    QHBoxLayout *hlayout_reader = new QHBoxLayout;
    hlayout_reader->addWidget(readerEdit);
    hlayout_reader->addWidget(AddReaderButton);

    QHBoxLayout *hlayout_control = new QHBoxLayout;
    hlayout_control->addWidget(controlEdit);
    hlayout_control->addWidget(AddControlButton);

    QHBoxLayout *hlayout1 = new QHBoxLayout;
    hlayout1->addStretch(1200);
    hlayout1->addWidget(saveButton);
    hlayout1->addWidget(cancelButton);

    QVBoxLayout *vlayout1 = new QVBoxLayout;
    vlayout1->addWidget(readerLabel);
    vlayout1->addLayout(hlayout_reader);
    vlayout1->addSpacing(20);
    vlayout1->addWidget(controlLabel);
    vlayout1->addLayout(hlayout_control);
    vlayout1->addSpacing(20);
    vlayout1->addLayout(hlayout1);

    edit_program_dialog->setLayout(vlayout1);
    edit_program_dialog->adjustSize();

    connect(cancelButton,   SIGNAL(clicked()), edit_program_dialog, SLOT(close()));
    connect(AddReaderButton, SIGNAL(clicked()), this,SLOT(add_reader()));
    connect(AddControlButton, SIGNAL(clicked()), this,SLOT(add_control()));
    connect(saveButton,   SIGNAL(clicked()), this, SLOT(saveAndClose()));

    edit_program_dialog->exec();
}

void externalprogramlist::add_reader(){
    add_program("reader");
}

void externalprogramlist::add_control(){
    add_program("control");
}

void externalprogramlist::add_program(QString program){

    QString temp = QFileDialog::getOpenFileName(0, tr("Choose EEG"), mainwindow->defaultReaderFolder, tr("BrainLab(*.exe)"));

    if(temp.isEmpty()){
        return;
    }

    if(program == "reader"){
        readerEdit->setText(temp);
    }

    if(program == "control"){
        controlEdit->setText(temp);
    }
}

void externalprogramlist::saveAndClose(){
    // TO DO - path validator
    mainwindow->externalProgram1 = readerEdit->text();
    mainwindow->externalProgram2 = controlEdit->text();
    edit_program_dialog->close();
}
