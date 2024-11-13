#include "ui/JDObjectListWidget.h"
#include "utilities/ResourceManager.h"

#include <QVBoxLayout>

namespace JsonDatabase
{
	namespace UI
	{
		const QString JDObjectListWidget::s_iconLock = "lock.png";
		const QString JDObjectListWidget::s_iconUnlock = "unlock.png";

		JDObjectListWidget::JDObjectListWidget(JDManager* manager, QWidget* parent)
			: QWidget(parent)
			, m_manager(manager)
		{
			connect(m_manager, &JDManager::objectLocked, this, &JDObjectListWidget::onObjectLocked);
			connect(m_manager, &JDManager::objectUnlocked, this, &JDObjectListWidget::onObjectUnlocked);
			setupUI();
		}
		JDObjectListWidget::~JDObjectListWidget()
		{

		}


		/*void JDObjectListWidget::setObjects(const std::vector<JDObject>& objects)
		{
			JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
			const QIcon& lockIcon = Utilities::ResourceManager::getIcon(s_iconLock);
			const QIcon& unlockIcon = Utilities::ResourceManager::getIcon(s_iconUnlock);
			//m_objects = objects;
			std::vector<JDObject> addList;
			std::vector<JDObject> removeList;
			for (const JDObject& object : objects)
			{
				if (findObjectIndex(object) == -1)
					addList.push_back(object);
			}
			for (const JDObjectItem& object : m_objects)
			{
				if (std::find(objects.begin(), objects.end(), object.object) == objects.end())
					removeList.push_back(object.object);
			}

			m_objects.reserve(m_objects.size() + addList.size());
			for (const JDObject& object : addList)
			{
				JDObjectItem item;
				item.object = object;
				item.item = new QStandardItem(object->className().c_str());
				if (object->isLocked())
					item.item->setIcon(lockIcon);
				else
					item.item->setIcon(unlockIcon);
				m_model->appendRow(item.item);
				m_objects.push_back(item);
			}

			for (const JDObject& object : removeList)
			{
				size_t index = findObjectIndex(object);
				if (index != -1)
				{
					m_model->removeRow(index);
					m_objects.erase(m_objects.begin() + index);
				}
			}


			//updateUI();
		}*/

		void JDObjectListWidget::update()
		{
			JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
			const QIcon& lockIcon = Utilities::ResourceManager::getIcon(s_iconLock);
			const QIcon& unlockIcon = Utilities::ResourceManager::getIcon(s_iconUnlock);

			const auto& objs = m_manager->getObjects();
			std::vector<JDObject> lockedObjs;
			Error err;
			m_manager->getLockedObjects(lockedObjs, err);
			std::vector<JDObject> addList;
			std::vector<JDObject> removeList;
			for (const JDObject& object : objs)
			{
				if (findObjectIndex(object) == -1)
					addList.push_back(object);
			}
			for (const JDObjectItem& object : m_objects)
			{
				if (std::find(objs.begin(), objs.end(), object.object) == objs.end())
					removeList.push_back(object.object);
				object.item->setIcon(unlockIcon);
			}

			m_objects.reserve(m_objects.size() + addList.size());
			for (const JDObject& object : addList)
			{
				JDObjectItem item;
				item.object = object;
				item.item = new QStandardItem(object->className().c_str());
				if (object->isLocked())
					item.item->setIcon(lockIcon);
				else
					item.item->setIcon(unlockIcon);
				m_model->appendRow(item.item);
				m_objects.push_back(item);
			}

			for (const JDObject& object : removeList)
			{
				size_t index = findObjectIndex(object);
				if (index != -1)
				{
					m_model->removeRow(index);
					m_objects.erase(m_objects.begin() + index);
				}
			}



			for (size_t i=0; i< lockedObjs.size(); ++i)
			{
				size_t index = findObjectIndex(lockedObjs[i]);
				if (index != -1)
				{
					m_objects[index].item->setIcon(lockIcon);
				}
			}
		}

		void JDObjectListWidget::onObjectLocked(JDObject obj)
		{
			const QIcon& lockIcon = Utilities::ResourceManager::getIcon(s_iconLock);


			size_t index = findObjectIndex(obj);
			if (index != -1)
			{
				m_objects[index].item->setIcon(lockIcon);
			}
		}
		void JDObjectListWidget::onObjectUnlocked(JDObject obj)
		{
			const QIcon& unlockIcon = Utilities::ResourceManager::getIcon(s_iconUnlock);

			size_t index = findObjectIndex(obj);
			if (index != -1)
			{
				m_objects[index].item->setIcon(unlockIcon);
			}
		}
		
		void JDObjectListWidget::setupUI()
		{
			m_objectListWidget = new QListView(this);
			m_objectListWidget->setIconSize(QSize(48, 48)); // Set the icon size if needed

			m_model = new QStandardItemModel(this);

			QVBoxLayout* layout = new QVBoxLayout(this);
			layout->addWidget(m_objectListWidget);
			setLayout(layout);
			m_objectListWidget->setModel(m_model);
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
		size_t JDObjectListWidget::findObjectIndex(const JDObject& object) const
		{
			for (size_t i = 0; i < m_objects.size(); ++i)
			{
				if (m_objects[i].object == object)
					return i;
			}
			return -1;
		}
	}
}