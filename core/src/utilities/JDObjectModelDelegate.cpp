#include "utilities/JDObjectModelDelegate.h"
#include "utilities/JDObjectItemModel.h"
#include "utilities/ResourceManager.h"
#include <QToolTip>

namespace JsonDatabase
{
    namespace Utilities
    {
        JDObjectModelDelegate::JDObjectModelDelegate(JDObjectItemModel* model, QAbstractItemView *view)
            : QStyledItemDelegate(model)
			, m_model(model)
            , m_view(view)
        {

        }

        // Custom size hint for items
        QSize JDObjectModelDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
        {
            JDObject obj = m_model->getObject(index);
            if (!obj)
            {
                QSize defaultSize = QStyledItemDelegate::sizeHint(option, index);
                return QSize(defaultSize.width(), 5);
            }
            return obj->getSizeHint();
        }

        // Custom painting for items
        void JDObjectModelDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
        {
            JDObject obj = m_model->getObject(index);
            if (!obj)
                return;

            QString text = obj->getDisplayName().c_str();

            painter->save();

            QIcon objIcon = obj->getIcon();
            QColor objColor = obj->getColor();
            bool hasIcon = !objIcon.isNull();
            QRect iconRect = option.rect;
            int iconSize = option.rect.height();
            iconRect.setWidth(iconSize);
            int infoIconSize = std::min(32, option.rect.height());
            QRect lockIconRect(option.rect.width() - infoIconSize, option.rect.top(), infoIconSize, infoIconSize);
            // Convert global mouse position to view-local coordinates
            QPoint mousePos = m_view->viewport()->mapFromGlobal(QCursor::pos());
            bool showToolTip = option.rect.contains(mousePos);


            // Apply selection color if selected
            if (option.state & QStyle::State_Selected)
            {
                painter->fillRect(option.rect, option.palette.highlight());
                painter->setPen(option.palette.highlightedText().color());
            }

            if (hasIcon)
            {
                painter->drawPixmap(iconRect.x(), iconRect.y(), objIcon.pixmap(iconRect.width(), iconRect.height()));
            }

            int infoIconXPos = lockIconRect.left();

            if (m_model->isLocked(obj))
            {
                const QIcon& lockIcon = obj->getStatusIcon(JDObjectInterface::Status::Locked);
                painter->drawPixmap(lockIconRect.x(), lockIconRect.y(), lockIcon.pixmap(lockIconRect.width(), lockIconRect.height()));
                infoIconXPos -= infoIconSize;
                if (showToolTip)
                    if (lockIconRect.contains(mousePos))
                    {
                        showToolTip = false;
                        QToolTip::showText(QCursor::pos(), obj->getLockedToolTip().c_str(), m_view);
                    }
            }

			
            if (obj->hasChanges())
            {
                QRect unsavedIconRect = lockIconRect;
                unsavedIconRect.moveLeft(infoIconXPos);
				infoIconXPos -= infoIconSize;
               
				const QIcon& unsavedIcon = obj->getStatusIcon(JDObjectInterface::Status::UnsavedChanges);
				painter->drawPixmap(unsavedIconRect.x(), unsavedIconRect.y(), unsavedIcon.pixmap(unsavedIconRect.width(), unsavedIconRect.height()));
                if(showToolTip)
                    if (unsavedIconRect.contains(mousePos))
                    {
                        showToolTip = false;
					    QToolTip::showText(QCursor::pos(), obj->getUnsavedChangesToolTip().c_str(), m_view);
                    }
            }
            if (obj->hasWrongData())
            {
                QRect wrongDataIconRect = lockIconRect;
                wrongDataIconRect.moveLeft(infoIconXPos);
                infoIconXPos -= infoIconSize;
                const QIcon& wrongDataIcon = obj->getStatusIcon(JDObjectInterface::Status::WrongData);
				painter->drawPixmap(wrongDataIconRect.x(), wrongDataIconRect.y(), wrongDataIcon.pixmap(wrongDataIconRect.width(), wrongDataIconRect.height()));
                if (showToolTip)
                    if (wrongDataIconRect.contains(mousePos))
                    {
                        showToolTip = false;
                        QToolTip::showText(QCursor::pos(), obj->getWrongDataToolTip().c_str(), m_view);
                    }
            }
            if (obj->markedForRemoval())
            {
                QRect removalIconRect = lockIconRect;
                removalIconRect.moveLeft(infoIconXPos);
                infoIconXPos -= infoIconSize;
				const QIcon& removalIcon = obj->getStatusIcon(JDObjectInterface::Status::MarkedForRemoval);
                painter->drawPixmap(removalIconRect.x(), removalIconRect.y(), removalIcon.pixmap(removalIconRect.width(), removalIconRect.height()));
                if (showToolTip)
                    if (removalIconRect.contains(mousePos))
                    {
                        showToolTip = false;
                        QToolTip::showText(QCursor::pos(), obj->getMarkedForRemovalToolTip().c_str(), m_view);
                    }
            }

            // Draw the text
            QRect textRect = option.rect;
            textRect.setX(option.rect.x() + iconSize);
            painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, text);

            if (showToolTip)
            {
                QToolTip::showText(QCursor::pos(), obj->getToolTip().c_str(), m_view);
            }

            painter->restore();
        }
    }
}