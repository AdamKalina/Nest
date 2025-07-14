#ifndef LEAFFILTERPROXYMODEL_H
#define LEAFFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QColor>
#include <QDebug>
//#include "mainwindow.h"

class MainWindow;

class LeafFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit LeafFilterProxyModel(QObject *parent = nullptr);
    MainWindow *mainwindow;

signals:
    void callParentFetchSignal();


public slots:
    void emitFetchMore();


protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
    bool filterAcceptsRowItself(int source_row, const QModelIndex &source_parent) const;
    bool hasAcceptedChildren(int source_row, const QModelIndex &source_parent) const;
    //virtual QVariant data( const QModelIndex &index, int role ) const;
    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;

};
#endif // LEAFFILTERPROXYMODEL_H
