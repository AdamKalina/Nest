#include "treemodel.h"

TreeModel::TreeModel(QObject *w_parent)
{

    mainwindow = (MainWindow *)w_parent;

    QStandardItemModel *model = new QStandardItemModel(0, 7, w_parent);

    model->setHeaderData(0, Qt::Horizontal, QString::fromLocal8Bit("Rodné èíslo"));
    model->setHeaderData(1, Qt::Horizontal, QString::fromLocal8Bit("Jméno"));
    model->setHeaderData(2, Qt::Horizontal, QString::fromLocal8Bit("Poslední EEG"));
    model->setHeaderData(3, Qt::Horizontal, QString::fromLocal8Bit("Poèet EEG"));
    model->setHeaderData(4, Qt::Horizontal, QString::fromLocal8Bit("Cesta"));
    model->setHeaderData(5, Qt::Horizontal, QString::fromLocal8Bit("Recorded"));
    model->setHeaderData(6, Qt::Horizontal, QString::fromLocal8Bit("System"));
}

//int TreeModel::rowCount(const QModelIndex &parent) const
//{
//    TreeItem *parentItem;
//    if (parent.column() > 0)
//        return 0;

//    if (!parent.isValid())
//        parentItem = rootItem;
//    else
//        parentItem = static_cast<TreeItem*>(parent.internalPointer());

//    return parentItem->childCount();
//}

//int TreeModel::columnCount(const QModelIndex &parent) const
//{
//    if (parent.isValid())
//        return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
//    return rootItem->columnCount();
//}

//QVariant TreeModel::data(const QModelIndex &index, int role) const
//{
//    if (!index.isValid())
//        return QVariant();

//    if (role != Qt::DisplayRole)
//        return QVariant();

//    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

//    return item->data(index.column());
//}

//QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
//{
//    if (!hasIndex(row, column, parent))
//        return QModelIndex();

//    TreeItem *parentItem;

//    if (!parent.isValid())
//        parentItem = rootItem;
//    else
//        parentItem = static_cast<TreeItem*>(parent.internalPointer());

//    TreeItem *childItem = parentItem->child(row);
//    if (childItem)
//        return createIndex(row, column, childItem);
//    return QModelIndex();
//}

//QModelIndex TreeModel::parent(const QModelIndex &index) const
//{
//    if (!index.isValid())
//        return QModelIndex();

//    TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
//    TreeItem *parentItem = childItem->parentItem();

//    if (parentItem == rootItem)
//        return QModelIndex();

//    return createIndex(parentItem->row(), 0, parentItem);
//}

//bool TreeModel::canFetchMore(const QModelIndex & parent) const
//{
//    return true;
//}
////![1]

////![2]
//void TreeModel::fetchMore(const QModelIndex &parent)
//{
//    if (parent.isValid())
//        return;

//    int itemsToFetch = mainwindow->no_of_patients_load;

//    if (itemsToFetch <= 0)
//        return;

//    beginInsertRows(QModelIndex(), fileCount, fileCount + itemsToFetch - 1);

//    fileCount += itemsToFetch;

//    endInsertRows();

//    emit numberPopulated(itemsToFetch);
//}
