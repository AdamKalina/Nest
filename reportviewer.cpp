#include "reportviewer.h"

reportViewer::reportViewer(QWidget *w_parent)
{
    // Create the QTextEdit
    this->setParent(w_parent);
    reportView = new QTextEdit(this); // 'this' sets the window as the parent
    reportView->setReadOnly(true);

    QFont font = QFont ("Times");
    font.setPointSize (12);
    reportView->setFont(font);

    // Create a layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(reportView); // Add the QTextEdit to the layout

    // Set the layout for the window
    this->setLayout(layout);

    // Optionally set window properties
    this->setWindowTitle("EEG report");
    this->setFixedWidth(1500);
    this->setMinimumHeight(600);
}

void reportViewer::setText(QStringList report){
    qDebug() << report.at(9);

    QString reportQString = QString::fromLocal8Bit("<b>Indikace:</b> %1<br><br>"
    "<b>Úroveò vìdomí:</b> %2<br><br>"
    "<b>Popis</b>: %3<br><br>"
    "<b>Závìr:</b> %4<br><br>"
    "<b>Klinická interpretace:</b> %5").arg(report.at(8),report.at(9),report.at(11),report.at(14),report.at(15));

    this->setWindowTitle("EEG report - " + report.at(3) + " " + report.at(2) + " - " +  report.last());

    reportView->setText(reportQString);
    //reportView->adjustSize();
}
