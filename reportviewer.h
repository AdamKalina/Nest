#ifndef REPORTVIEWER_H
#define REPORTVIEWER_H

#include <QWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QDebug>

class reportViewer : public QWidget {
    Q_OBJECT
public:
    reportViewer(QWidget *parent=nullptr);
    void setText(QStringList report);

private:
    QTextEdit *reportView;
};

#endif // REPORTVIEWER_H
