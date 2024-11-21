#include "utilities/JDObjectItemModel.h"




namespace JsonDatabase
{
	namespace Utilities
	{
		

		JDObjectItemModel::JDObjectItemModel(JDManager *manager, QObject* parent)
			: QStandardItemModel(parent)
			, m_manager(manager)
		{
			connect(m_manager, &JDManager::objectAdded, this, &JDObjectItemModel::onObjectAdded);
			connect(m_manager, &JDManager::objectRemoved, this, &JDObjectItemModel::onObjectRemoved);
			connect(m_manager, &JDManager::objectLocked, this, &JDObjectItemModel::onObjectLocked);
			connect(m_manager, &JDManager::objectUnlocked, this, &JDObjectItemModel::onObjectUnlocked);
			connect(m_manager, &JDManager::lockedObjectsChanged, this, &JDObjectItemModel::onLockedObjectsChanged);

			connect(m_manager, &JDManager::destroyed, this, &JDObjectItemModel::onManagerDestroyed);

			const auto& objs = m_manager->getObjects();
			m_objects.reserve(objs.size());
			for (const JDObject& obj : objs)
			{
				onObjectAdded(obj);
			}
		}
		JDObjectItemModel::~JDObjectItemModel()
		{

		}

		JDObject JDObjectItemModel::getObject(const QModelIndex& index) const
		{
			if (index.isValid())
			{
				size_t row = index.row();
				if (row < m_objects.size())
				{
					return m_objects[row].object;
				}
			}
			return nullptr;
		}
		QModelIndex JDObjectItemModel::getIndex(const JDObject& obj) const
		{
			size_t index = findObjectIndex(obj);
			if (index != (size_t)(-1))
			{
				return m_objects[index].item->index();
			}
			return QModelIndex();
		}

		QVariant JDObjectItemModel::data(const QModelIndex& index, int role) const
		{
			JDObject obj = getObject(index);
			if (!obj)
				return QVariant();
			switch (role)
			{
			/*case Qt::DisplayRole:
			{
				QString text = obj->getDisplayName().c_str();
				auto id = obj->getShallowObjectID();
				const auto& it = m_lockedObjects.find(id);
				if (it != m_lockedObjects.end())
				{
					text += " (Locked by " + QString(it->second.lockData.user.getName().c_str()) + ")";
				}
				return text;
			}*/
				case Qt::ToolTipRole:	return obj->getToolTip().c_str();
			}
			return QStandardItemModel::data(index, role);
		}

		void JDObjectItemModel::onManagerDestroyed()
		{
			this->clear();
			m_manager = nullptr;
		}

		void JDObjectItemModel::onObjectAdded(JDObject obj)
		{
			JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
			if (findObjectIndex(obj) != (size_t)(-1))
				return;
			JDObjectItem item;
			item.object = obj;
			item.item = new QStandardItem();
			// Disable edit mode for the item
			item.item->setFlags(item.item->flags() & ~Qt::ItemIsEditable);

			
			this->appendRow(item.item);
			m_objects.push_back(item);
			updateObjectItemLockState(m_objects.size() - 1, obj->isLocked());
		}
		void JDObjectItemModel::onObjectRemoved(JDObject obj)
		{
			JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
			size_t index = findObjectIndex(obj);
			if (index != -1)
			{
				this->removeRow(index);
				m_objects.erase(m_objects.begin() + index);
			}
		}
		void JDObjectItemModel::onObjectLocked(JDObject obj)
		{
			JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
			size_t index = findObjectIndex(obj);
			if (index != -1)
			{
			//	updateObjectItemLockState(index, true);
			}
		}
		void JDObjectItemModel::onObjectUnlocked(JDObject obj)
		{
			JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
			size_t index = findObjectIndex(obj);
			if (index != -1)
			{
			//	updateObjectItemLockState(index, false);
			}
		}
		void JDObjectItemModel::onLockedObjectsChanged()
		{
			JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
			for (const auto &lobj : m_lockedObjects)
			{
				size_t index = findObjectIndex(lobj.second.obj);
				if (index != -1)
				{
					updateObjectItemLockState(index, false);
				}
			}
			m_lockedObjects.clear();
			std::vector<JDManager::LockedObject> lockedObjects;
			Error err;
			if (m_manager->getLockedObjects(lockedObjects, err))
			{
				for (const JDManager::LockedObject& obj : lockedObjects)
				{
					size_t index = findObjectIndex(obj.obj);
					if (index != -1)
					{
						m_lockedObjects[obj.obj->getObjectID()->get()] = obj;
						updateObjectItemLockState(index, true);
					}
				}
			}
		}
		void JDObjectItemModel::updateObjectItemLockState(size_t index, bool isLocked)
		{
			index;
			isLocked;
			/*JDObjectItem& item = m_objects[index];
			if (isLocked)
			{
				item.item->setIcon(Utilities::ResourceManager::getIcon(s_iconLock));
				item.item->setBackground(QBrush(QColor(255, 0, 0, 100)));
			}
			else
			{
				item.item->setIcon(Utilities::ResourceManager::getIcon(s_iconUnlock));
				item.item->setBackground(QBrush(QColor(0, 255, 0, 100)));
			}*/
		}
		size_t JDObjectItemModel::findObjectIndex(const JDObject& object) const
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