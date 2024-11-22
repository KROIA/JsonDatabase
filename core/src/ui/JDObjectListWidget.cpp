#include "ui/JDObjectListWidget.h"


#include <QVBoxLayout>

namespace JsonDatabase
{
	namespace UI
	{
		

		JDObjectListWidget::JDObjectListWidget(JDManager* manager, QWidget* parent)
			: QWidget(parent)
			, m_manager(manager)
		{
			
			
			setupUI();
		}
		JDObjectListWidget::~JDObjectListWidget()
		{

		}

		void JDObjectListWidget::onItemClicked(const QModelIndex& index)
		{
			JDObject obj = m_model->getObject(index);
			if (obj)
			{
				emit objectClicked(obj);
			}
		}
		void JDObjectListWidget::onItemDoubleClicked(const QModelIndex& index)
		{
			JDObject obj = m_model->getObject(index);
			if (obj)
			{
				emit objectDoubleClicked(obj);
			}
		}
		
		void JDObjectListWidget::setupUI()
		{
			m_objectListWidget = new QListView(this);
			m_objectListWidget->setIconSize(QSize(48, 48)); // Set the icon size if needed

			m_model = new Utilities::JDObjectItemModel(m_manager, this);
			m_delegate = new Utilities::JDObjectModelDelegate(m_model);

			QVBoxLayout* layout = new QVBoxLayout(this);
			layout->addWidget(m_objectListWidget);
			setLayout(layout);
			m_objectListWidget->setModel(m_model);
			m_objectListWidget->setItemDelegate(m_delegate);
			connect(m_objectListWidget, &QListView::clicked, this, &JDObjectListWidget::onItemClicked);
			connect(m_objectListWidget, &QListView::doubleClicked, this, &JDObjectListWidget::onItemDoubleClicked);

		}
		void JDObjectListWidget::updateUI()
		{
			/*m_model->clear();
			const QIcon& lockIcon = Utilities::ResourceManager::getIcon("lock.png");
			const QIcon& unlockIcon = Utilities::ResourceManager::getIcon("unlock.png");

			for (const JDObject& object : m_objects)
			{
				QStandardItem* item = new QStandardItem(object->className().c_str());
				if (object->isLocked())
					item->setIcon(lockIcon);
				else
					item->setIcon(unlockIcon);
				m_model->appendRow(item);
			}

			m_objectListWidget->setModel(m_model);*/

		}
	}
}