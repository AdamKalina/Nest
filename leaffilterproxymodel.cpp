#include "leaffilterproxymodel.h"

// with courtesy of Andre - https://forum.qt.io/topic/7606/qsortfilterproxymodel-filteracceptsrow-filter-parent-node-and-child-node/4

LeafFilterProxyModel::LeafFilterProxyModel(QObject *w_parent) :
    QSortFilterProxyModel(w_parent)
{
    //connect(this, SIGNAL(callParentSignal()), SLOT(emitFetchMore())); // local signal
    connect(this, SIGNAL(callParentFetchSignal()), this->parent(), SLOT(fetchMorePatients()));  // signal the parent
}

bool LeafFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (filterAcceptsRowItself(source_row, source_parent))
        return true;

    //accept if any of the parents is accepted on it's own merits
    QModelIndex parent = source_parent;
    while (parent.isValid()) {
        if (filterAcceptsRowItself(parent.row(), parent.parent()))
            return true;
        parent = parent.parent();
    }

    //accept if any of the children is accepted on it's own merits
    if (hasAcceptedChildren(source_row, source_parent)) {
        return true;
    }

    return false;

}

bool LeafFilterProxyModel::filterAcceptsRowItself(int source_row, const QModelIndex &source_parent) const
{
    return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

bool LeafFilterProxyModel::hasAcceptedChildren(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex item = sourceModel()->index(source_row,0,source_parent);
    if (!item.isValid()) {
        //qDebug() << "item invalid" << source_parent << source_row;
        return false;
    }

    //check if there are children
    int childCount = item.model()->rowCount(item);
    if (childCount == 0)
        return false;

    for (int i = 0; i < childCount; ++i) {
        if (filterAcceptsRowItself(i, item))
            return true;
        //recursive call -> NOTICE that this is depth-first searching, you're probably better off with breadth first search...
        if (hasAcceptedChildren(i, item))
            return true;
    }

    return false;
}

bool LeafFilterProxyModel::canFetchMore(const QModelIndex &parent) const{
    return true;
}

void LeafFilterProxyModel::fetchMore(const QModelIndex &parent){
    //mainwindow->loadMorePatients();
    //mainwindow->updatePatientTreeModel();
    //emitFetchMore();
    emit callParentFetchSignal();
}

void LeafFilterProxyModel::emitFetchMore(){
    qDebug() << "got the local signal!";
}
