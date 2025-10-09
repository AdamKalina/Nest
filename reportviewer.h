#ifndef REPORTVIEWER_H
#define REPORTVIEWER_H

#include <QWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QDebug>
#include "qpatient.h"

class reportViewer : public QWidget {
    Q_OBJECT
public:
    reportViewer(QWidget *parent=nullptr);
    void setText(QReport qreport);

private:
    QTextEdit *reportView;
};

#endif // REPORTVIEWER_H
