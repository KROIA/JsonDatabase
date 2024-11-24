#pragma once

#include "JsonDatabase_base.h"
#include <QStyledItemDelegate>
#include <QPainter>

namespace JsonDatabase
{
    namespace Utilities
    {
        class JDObjectItemModel;
        class JSON_DATABASE_EXPORT JDObjectModelDelegate: public QStyledItemDelegate 
        {
            Q_OBJECT

            public:
            explicit JDObjectModelDelegate(JDObjectItemModel *model);


			QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

            // Custom painting for items
            void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
        
			static void setLockColor(const QColor& color)
			{
				s_colorLock = color;
			}
			static void setUnlockColor(const QColor& color)
			{
				s_colorUnlock = color;
			}
			static void useLockColor(bool use)
			{
				s_useLockColor = use;
			}
			void setItemHeight(unsigned int height)
			{
				s_itemHeight = height;
			}
			const QColor& getLockColor() const
			{
				return s_colorLock;
			}
			const QColor& getUnlockColor() const
			{
				return s_colorUnlock;
			}
			bool isUsingLockColor() const
			{
				return s_useLockColor;
			}
			unsigned int getItemHeight() const
			{
				return s_itemHeight;
			}
            private:
			JDObjectItemModel* m_model;

			static QColor s_colorLock;
			static QColor s_colorUnlock;
			static bool s_useLockColor;
			static unsigned int s_itemHeight;

        };
    }
}
