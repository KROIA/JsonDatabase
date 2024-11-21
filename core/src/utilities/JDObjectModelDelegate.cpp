#include "utilities/JDObjectModelDelegate.h"
#include "utilities/JDObjectItemModel.h"
#include "utilities/ResourceManager.h"

namespace JsonDatabase
{
    namespace Utilities
    {
        QColor JDObjectModelDelegate::s_colorLock = QColor(255,0,0,100);
        QColor JDObjectModelDelegate::s_colorUnlock = QColor(0,255,0,100);
        bool JDObjectModelDelegate::s_useLockColor = true;
		unsigned int JDObjectModelDelegate::s_itemHeight = 80;

        const QString JDObjectModelDelegate::s_iconLock = "lock.png";
        const QString JDObjectModelDelegate::s_iconUnlock = "unlock.png";

        JDObjectModelDelegate::JDObjectModelDelegate(JDObjectItemModel* model)
            : QStyledItemDelegate(model)
			, m_model(model)
        {

        }

        // Custom size hint for items
        QSize JDObjectModelDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
        {
            QSize defaultSize = QStyledItemDelegate::sizeHint(option, index);
            return QSize(defaultSize.width(), s_itemHeight);
        }

        // Custom painting for items
        void JDObjectModelDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const 
        {
            
			JDObject obj = m_model->getObject(index);
            if (!obj)
                return;
			bool isLocked = m_model->isLocked(obj);


            QString text = obj->getDisplayName().c_str();
            if (isLocked)
            {
				auto lockData = m_model->getLockedObjectData(obj);
                text += " (Locked by " + QString(lockData.lockData.user.getName().c_str()) + ")";
            }
            painter->save();


			QIcon objIcon = obj->getIcon();
			QColor objColor = obj->getColor();
			bool hasIcon = !objIcon.isNull();
			QRect iconRect = option.rect;
            int iconSize = option.rect.height();
			iconRect.setWidth(iconSize);
			QRect lockIconRect = iconRect;

			painter->fillRect(option.rect, objColor);

            if (hasIcon)
            {
                // Lock icon is at the top right corner of the objects icon
                lockIconRect.setWidth(iconSize / 2);
                lockIconRect.setHeight(iconSize / 2);
                lockIconRect.moveTop(iconRect.top());
                lockIconRect.moveLeft(iconRect.right()- lockIconRect.width());
                painter->drawPixmap(iconRect.x(), iconRect.y(), objIcon.pixmap(iconRect.width(), iconRect.height()));
            }

          
			// Draw Icon and then text
            
			if (isLocked)
			{
                if(s_useLockColor)
                    painter->fillRect(option.rect, s_colorLock);
                const QIcon& lockIcon = Utilities::ResourceManager::getIcon(s_iconLock);
                painter->drawPixmap(lockIconRect.x(), lockIconRect.y(), lockIcon.pixmap(lockIconRect.width(), lockIconRect.height()));
			}
			else
			{
                if(s_useLockColor)
                    painter->fillRect(option.rect, s_colorUnlock);
                const QIcon& lockIcon = Utilities::ResourceManager::getIcon(s_iconUnlock);
                painter->drawPixmap(lockIconRect.x(), lockIconRect.y(), lockIcon.pixmap(lockIconRect.width(), lockIconRect.height()));
			}

            // Draw the text
            painter->setPen(Qt::black);
			QRect textRect = option.rect;
			textRect.setX(option.rect.x() + iconSize);
            painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, text);

            painter->restore();
        }
    }
}