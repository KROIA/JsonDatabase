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
			//connect(m_manager, &JDManager::objectLocked, this, &JDObjectItemModel::onObjectLocked);
			//connect(m_manager, &JDManager::objectUnlocked, this, &JDObjectItemModel::onObjectUnlocked);
			connect(m_manager, &JDManager::lockedObjectsChanged, this, &JDObjectItemModel::onLockedObjectsChanged);

			connect(m_manager, &JDManager::destroyed, this, &JDObjectItemModel::onManagerDestroyed);

			const auto& objs = m_manager->getObjects();
			m_objects.reserve(objs.size());
			
			beginInsertRows(QModelIndex(), 0, objs.size());
			for (const JDObject& obj : objs)
			{
				addObject_iternal(obj);
			}
			setSorter(std::make_shared<AlphabeticalSorter>());
			endInsertRows();
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
				case Qt::ToolTipRole:	return obj->getToolTip().c_str();
			}
			return QStandardItemModel::data(index, role);
		}

		void JDObjectItemModel::clearFilters()
		{
			m_filters.clear();
			onFilterChanged();
		}
		void JDObjectItemModel::setFilter(std::shared_ptr<Filter> filter)
		{
			m_filters.clear();
			m_filters.push_back(filter);
			onFilterChanged();
		}
		void JDObjectItemModel::setFilter(const std::vector<std::shared_ptr<Filter>>& filters)
		{
			m_filters = filters;
			onFilterChanged();
		}
		void JDObjectItemModel::addFilter(std::shared_ptr<Filter> filter)
		{
			// check if filter already exists
			for (const auto& f : m_filters)
			{
				if (f == filter)
					return;
			}
			m_filters.push_back(filter);
			onFilterChanged();
		}
		void JDObjectItemModel::removeFilter(std::shared_ptr<Filter> filter)
		{
			auto it = std::find(m_filters.begin(), m_filters.end(), filter);
			if (it != m_filters.end())
			{
				m_filters.erase(it);
				onFilterChanged();
			}
		}

		void JDObjectItemModel::setSorter(std::shared_ptr<Sorter> sorter)
		{
			m_sorter = sorter;
			onSort();
		}
		
		void JDObjectItemModel::onFilterChanged()
		{
			JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
			beginResetModel();
			std::vector<JDObject> objs = m_manager->getObjects();
			std::vector<JDObject> objsFiltered;
			filterObjects(objs, objsFiltered);
			JD_GENERAL_PROFILING_BLOCK("Removing objects from model", JD_COLOR_STAGE_2);
			QStandardItemModel::clear();
			m_objects.clear();
			JD_GENERAL_PROFILING_END_BLOCK;
			JD_GENERAL_PROFILING_BLOCK("Adding objects to model", JD_COLOR_STAGE_2);
			for (const JDObject& obj : objsFiltered)
			{
				if (findObjectIndex(obj) == (size_t)(-1))
				{
					addObject_iternal(obj);
				}
			}
			JD_GENERAL_PROFILING_END_BLOCK;
			onSort();
			endResetModel();
		}
		void JDObjectItemModel::onSort()
		{
			JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
			if (m_sorter)
			{
				beginResetModel();
				std::sort(m_objects.begin(), m_objects.end(), [this](const JDObjectItem& a, const JDObjectItem& b) -> bool
						  {
							  return m_sorter->sort(a.object, b.object);
						  });
				endResetModel();
			}
		}
		void JDObjectItemModel::onManagerDestroyed()
		{
			this->clear();
			m_manager = nullptr;
		}

		void JDObjectItemModel::onObjectAdded(std::vector<JDObject> objs)
		{
			std::vector<JDObject> objsFiltered;
			filterObjects(objs, objsFiltered);
			if (objsFiltered.size() > 0)
			{
				beginInsertRows(QModelIndex(), m_objects.size(), m_objects.size()+ objsFiltered.size());
				for (size_t i = 0; i < objsFiltered.size(); ++i)
				{
					if (findObjectIndex(objsFiltered[i]) == (size_t)(-1))
						addObject_iternal(objsFiltered[i]);
				}
				endInsertRows();
				onSort();
			}
		}
		void JDObjectItemModel::onObjectRemoved(std::vector<JDObject> objs)
		{
			beginRemoveRows(QModelIndex(), 0, m_objects.size());
			for (size_t i = 0; i < objs.size(); ++i)
				removeObject_internal(objs[i]);
			endRemoveRows();
		}
		/*void JDObjectItemModel::onObjectLocked(JDObject obj)
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
		}*/
		void JDObjectItemModel::onLockedObjectsChanged()
		{
			JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
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
					}
				}
			}
		}

		void JDObjectItemModel::addObject_iternal(const JDObject& obj)
		{
			JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
			JDObjectItem item;
			item.object = obj;
			item.item = new QStandardItem();
			// Disable edit mode for the item
			item.item->setFlags(item.item->flags() & ~Qt::ItemIsEditable);


			this->appendRow(item.item);
			m_objects.push_back(item);
		}
		void JDObjectItemModel::removeObject_internal(const JDObject& obj)
		{
			JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
			size_t index = findObjectIndex(obj);
			if (index != -1)
			{
				this->removeRow(index);
				m_objects.erase(m_objects.begin() + index);
			}
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
		bool JDObjectItemModel::filterObject(const JDObject& obj) const
		{
			JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
			if (m_filters.size() == 0)
				return true;
			for (const auto& filter : m_filters)
			{
				if (filter->filter(obj))
					return true;
			}
			return false;
		}
		void JDObjectItemModel::filterObjects(const std::vector<JDObject>& objs, std::vector<JDObject>& objsOut) const
		{
			JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
			if (m_filters.size() == 0)
			{
				objsOut = objs;
				return;
			}
			objsOut.clear();
			objsOut.reserve(objs.size());
			for (const JDObject& obj : objs)
			{
				if (filterObject(obj))
				{
					objsOut.push_back(obj);
				}
			}
		}
	}
}