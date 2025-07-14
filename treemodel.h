#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <QAbstractItemModel>
#include <QStandardItemModel>
#include "mainwindow.h"

class MainWindow;

class TreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit TreeModel(QObject *parent = nullptr);
    MainWindow *mainwindow;

//    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
//    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override; //index, parent, columnCount

//    int columnCount(const QModelIndex &parent) const override;
//    QModelIndex index (int row, int column, const QModelIndex & parent  ) const override;
//    QModelIndex parent ( const QModelIndex & index ) const override;

//protected:
//    bool canFetchMore(const QModelIndex &parent) const override;
//    void fetchMore(const QModelIndex &parent) override;

};

#endif // TREEMODEL_H
