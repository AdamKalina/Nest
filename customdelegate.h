#ifndef CUSTOMDELEGATE_H
#define CUSTOMDELEGATE_H

#include <QStyledItemDelegate>
#include <QPainter>
#include <QDebug>

class CustomDelegate : public QStyledItemDelegate
{

    Q_OBJECT

public:
    //using QStyledItemDelegate::QStyledItemDelegate; // this does not work for XP build
    CustomDelegate(QWidget *parent = 0) : QStyledItemDelegate(parent) {} // this needs to be here or "Compiler Error C2876" will happen in XP build
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // CUSTOMDELEGATE_H
