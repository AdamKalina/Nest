#include "customdelegate.h"

// TO DO - make the CustomDelegate use the same colors for MouseOver and Selected as QStyledItemDelegate

void CustomDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{

    if(!index.sibling(index.row(),4).data().isValid()){ // !path.isValid() = parent row, hasChildren() would choose just the the cell with the name
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        opt.font.setBold(true);
        QStyledItemDelegate::paint(painter, opt, index);
    }
    else{ // is children
        // if the file is still being recorded - color it red
        if (index.sibling(index.row(),5).data().toBool()){
            //painter->save();

            // need to reimplement color for Selected and MouseOver
            if (option.state & QStyle::State_Selected){
                //painter->fillRect(option.rect, QColor(204, 231, 254, 255));
                painter->setPen(Qt::white);
                painter->setBrush(option.palette.highlightedText());
            }

            if (option.state & QStyle::State_MouseOver)
                painter->fillRect(option.rect, QColor(204, 231, 254, 255));

            painter->setPen(Qt::red);
            painter->drawText(option.rect, Qt::AlignLeft|Qt::AlignVCenter, displayText(index.data(),QLocale::system()));
            //painter->restore();
            // otherwise use base QStyledItemDelegate
        } else { // nothing was changed, use normal style
            QStyledItemDelegate::paint(painter, option, index);
        }
    }
}
