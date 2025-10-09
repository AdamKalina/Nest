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
    this->setMinimumWidth(1000);
    this->setMinimumHeight(600);
}

void reportViewer::setText(QReport qreport){

    QString reportString;

    if(qreport.odesilajici_lekar_original != ""){
        reportString.append(QString::fromLocal8Bit("<b>Odesílající lékaø:</b> %1<br><br>").arg(qreport.odesilajici_lekar_original));
    }
    if(qreport.laborant != ""){
        reportString.append(QString::fromLocal8Bit("<b>Laborant:</b> %1<br><br>").arg(qreport.laborant));
    }
    if(qreport.lateralita != ""){
        reportString.append(QString::fromLocal8Bit("<b>Lateralita:</b> %1<br><br>").arg(qreport.lateralita));
    }
    if(qreport.duvod_vysetreni != ""){
        reportString.append(QString::fromLocal8Bit("<b>Indikace:</b> %1<br><br>").arg(qreport.duvod_vysetreni));
    }
    if(qreport.uroven_vedomi != ""){
        reportString.append(QString::fromLocal8Bit("<b>Úroveò vìdomí:</b> %1<br><br>").arg(qreport.uroven_vedomi));
    }
    if(qreport.popis != ""){
        reportString.append(QString::fromLocal8Bit("<b>Popis:</b> %1<br><br>").arg(qreport.popis));
    }
    if(qreport.fotostimulace != ""){
        reportString.append(QString::fromLocal8Bit("<b>Fotostimulace:</b> %1<br><br>").arg(qreport.fotostimulace));
    }
    if(qreport.tf != ""){
        reportString.append(QString::fromLocal8Bit("<b>TF:</b> %1<br><br>").arg(qreport.tf));
    }
    if(qreport.zaver_klasifikace != ""){
        reportString.append(QString::fromLocal8Bit("<b>Závìr:</b> %1<br><br>").arg(qreport.zaver_klasifikace));
    }
    if(qreport.klinicka_interpretace != ""){
        reportString.append(QString::fromLocal8Bit("<b>Klinická interpretace:</b> %1<br><br>").arg(qreport.klinicka_interpretace));
    }
    if(qreport.statisticky_kod_text != ""){
        reportString.append(QString::fromLocal8Bit("<b>Statistický kód:</b> %1<br><br>").arg(qreport.statisticky_kod_text));
    }

    this->setWindowTitle("EEG report - " + qreport.jmeno + " " + qreport.rodne_cislo + " - " +  qreport.datum);

    reportView->setText(reportString);
    //reportView->adjustSize();
}
