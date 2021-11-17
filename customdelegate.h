#ifndef CUSTOMDELEGATE_H
#define CUSTOMDELEGATE_H

#include <QStyledItemDelegate>
#include "mainwindow.h"

class CustomDelegate : public QStyledItemDelegate
{

    Q_OBJECT

public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    //QString displayText(const QVariant& value, const QLocale& locale) const;

};

#endif // CUSTOMDELEGATE_H
