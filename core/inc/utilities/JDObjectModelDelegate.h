#pragma once

#include "JsonDatabase_base.h"
#include <QStyledItemDelegate>
#include <QPainter>

namespace JsonDatabase
{
    namespace Utilities
    {
        class JDObjectItemModel;
        class JSON_DATABASE_API JDObjectModelDelegate: public QStyledItemDelegate 
        {
            Q_OBJECT

            public:
            explicit JDObjectModelDelegate(JDObjectItemModel *model, QAbstractItemView *view);


			QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

            // Custom painting for items
            void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
       
            private:
			JDObjectItemModel* m_model;
			QAbstractItemView* m_view;
        };
    }
}
