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

            //if (isLocked)
            //{
            //    auto lockData = m_model->getLockedObjectData(obj);
            //    text += " (Locked by " + QString(lockData.lockData.user.getName().c_str()) + ")";
            //}

            painter->save();

            QIcon objIcon = obj->getIcon();
            QColor objColor = obj->getColor();
            bool hasIcon = !objIcon.isNull();
            QRect iconRect = option.rect;
            int iconSize = option.rect.height();
            iconRect.setWidth(iconSize);
            int infoIconSize = std::min(32, option.rect.height());
            QRect lockIconRect(option.rect.width() - infoIconSize, option.rect.top(), infoIconSize, infoIconSize);
            
            // Lock icon is at the top right corner of the object icon
           
            /*
            QRect lockIconRect = iconRect;
            lockIconRect.setWidth(iconSize / 2);
            lockIconRect.setHeight(iconSize / 2);
            lockIconRect.moveTop(iconRect.top());
            lockIconRect.moveLeft(iconRect.right() - lockIconRect.width());
            */
            

            // Apply selection color if selected
            if (option.state & QStyle::State_Selected)
            {
                painter->fillRect(option.rect, option.palette.highlight());
                painter->setPen(option.palette.highlightedText().color());
            }
            else
            {
                if (s_useLockColor)
                {
                    if (isLocked)
                        painter->fillRect(option.rect, s_colorLock);
                    else
                        painter->fillRect(option.rect, s_colorUnlock);
                }
                else
				{
					painter->fillRect(option.rect, objColor);
				}
            }

            if (hasIcon)
            {
                painter->drawPixmap(iconRect.x(), iconRect.y(), objIcon.pixmap(iconRect.width(), iconRect.height()));
            }

            // Draw Lock Icon
            const QIcon& lockIcon = isLocked
                ? Utilities::ResourceManager::getIcon(Utilities::ResourceManager::Icon::lock)
                : Utilities::ResourceManager::getIcon(Utilities::ResourceManager::Icon::unlock);

            painter->drawPixmap(lockIconRect.x(), lockIconRect.y(), lockIcon.pixmap(lockIconRect.width(), lockIconRect.height()));

			int infoIconXPos = lockIconRect.left() - infoIconSize;
            if (obj->hasChanges())
            {
                QRect unsavedIconRect = lockIconRect;
                unsavedIconRect.moveLeft(infoIconXPos);
				infoIconXPos -= infoIconSize;
               
				const QIcon& unsavedIcon = Utilities::ResourceManager::getIcon(Utilities::ResourceManager::Icon::asterisk);
				painter->drawPixmap(unsavedIconRect.x(), unsavedIconRect.y(), unsavedIcon.pixmap(unsavedIconRect.width(), unsavedIconRect.height()));
            }
            if (obj->hasWrongData())
            {
                QRect wrongDataIconRect = lockIconRect;
                wrongDataIconRect.moveLeft(infoIconXPos);
				const QIcon& wrongDataIcon = Utilities::ResourceManager::getIcon(Utilities::ResourceManager::Icon::warning);
				painter->drawPixmap(wrongDataIconRect.x(), wrongDataIconRect.y(), wrongDataIcon.pixmap(wrongDataIconRect.width(), wrongDataIconRect.height()));
            }

            // Draw the text
            QRect textRect = option.rect;
            textRect.setX(option.rect.x() + iconSize);
            painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, text);

            painter->restore();
        }
    }
}