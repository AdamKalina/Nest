#include "folderlist.h"

folderList::folderList(QWidget *w_parent)
{
    mainwindow = (MainWindow *)w_parent;

    int i;

    QFont f( "Arial", 10, QFont::Bold);

    edit_folders_dialog = new QDialog;
    edit_folders_dialog->setMinimumSize(800, 265);
    edit_folders_dialog->setWindowTitle("Edit searched folders");
    edit_folders_dialog->setModal(true);
    edit_folders_dialog->setAttribute(Qt::WA_DeleteOnClose, true);

    QLabel *dynamicLabel = new QLabel(tr("Dynamic folders"));
    dynamicLabel->setFont(f);
    folder_path_list = new QListWidget;
    folder_path_list->setSelectionBehavior(QAbstractItemView::SelectRows);
    folder_path_list->setSelectionMode(QAbstractItemView::SingleSelection);
    folder_path_list->setSpacing(1);

    for(i=0; i < mainwindow->dynamic_dirs.size(); i++)
    {
        new QListWidgetItem(mainwindow->dynamic_dirs[i], folder_path_list);
    }

    QLabel *staticLabel = new QLabel(tr("Static folders"));
    staticLabel->setFont(f);
    sfolder_path_list = new QListWidget;
    sfolder_path_list->setSelectionBehavior(QAbstractItemView::SelectRows);
    sfolder_path_list->setSelectionMode(QAbstractItemView::SingleSelection);
    sfolder_path_list->setSpacing(1);

    for(i=0; i < mainwindow->static_dirs.size(); i++)
    {
        new QListWidgetItem(mainwindow->static_dirs[i], sfolder_path_list);
    }

    CloseButton = new QPushButton;
    CloseButton->setText("Close");

    QHBoxLayout *hlayout1 = new QHBoxLayout;
    hlayout1->addStretch(1000);
    hlayout1->addWidget(CloseButton);

    QVBoxLayout *vlayout1 = new QVBoxLayout;
    vlayout1->addWidget(dynamicLabel);
    vlayout1->addWidget(folder_path_list, 1000);
    vlayout1->addWidget(staticLabel);
    vlayout1->addWidget(sfolder_path_list, 1000);
    vlayout1->addSpacing(20);
    vlayout1->addLayout(hlayout1);

    edit_folders_dialog->setLayout(vlayout1);

    connect(CloseButton,   SIGNAL(clicked()), edit_folders_dialog, SLOT(close()));
    //connect(folder_path_list, SIGNAL(rowClicked(QListWidgetItem*)), this,                          SLOT(rowClicked(QListWidgetItem *)));

    edit_folders_dialog->exec();


}
