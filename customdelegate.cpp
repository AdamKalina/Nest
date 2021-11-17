#include "customdelegate.h"

// TO DO - make the CustomDelegate use tha same colors for MouseOver and Selected as QStyledItemDelegate

void CustomDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.siblingAtColumn(5).data().toBool()) {
        //painter->save();

        if (option.state & QStyle::State_Selected)
                    painter->fillRect(option.rect, QColor(204, 231, 254, 255));

        if (option.state & QStyle::State_MouseOver)
                    painter->fillRect(option.rect, QColor(204, 231, 254, 255));

        painter->setPen(Qt::red);
        painter->drawText(option.rect, Qt::AlignLeft|Qt::AlignVCenter, displayText(index.data(),QLocale::system()));
        //painter->restore();
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}
