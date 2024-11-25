#pragma once

#include "JsonDatabase_base.h"
#include "object/JDObjectInterface.h"
#include "manager/JDManager.h"
#include <QStandardItemModel>
#include <QModelIndex>


namespace JsonDatabase
{
	namespace Utilities
	{
		class JSON_DATABASE_EXPORT JDObjectItemModel : public QStandardItemModel
		{
			Q_OBJECT
			public:
			class Filter
			{
				public:
				Filter() = default;
				virtual ~Filter() = default;

				/**
				 * @brief
				 * Checks if the object matches the filter or not
				 * @param obj
				 * @return true if the object matches the filter, otherwise false
				 */
				virtual bool filter(const JDObject& obj) const = 0;
			};
			class Sorter
			{
				public:
				Sorter() = default;
				virtual ~Sorter() = default;

				/**
				 * @brief
				 * Compare function which is used for std::sort
				 * @param obj1
				 * @param obj2
				 * @return
				 */
				virtual bool sort(const JDObject& obj1, const JDObject& obj2) const = 0;
			};
			class AlphabeticalSorter : public Sorter
			{
				public:
				AlphabeticalSorter() = default;
				virtual ~AlphabeticalSorter() = default;

				bool sort(const JDObject& obj1, const JDObject& obj2) const override
				{
					return obj1->getDisplayName() < obj2->getDisplayName();
				}
			};

			JDObjectItemModel(JDManager* manager, QObject* parent = nullptr);
			~JDObjectItemModel();

			JDObject getObject(const QModelIndex& index) const;
			QModelIndex getIndex(const JDObject& obj) const;

			// Override data function
			QVariant data(const QModelIndex& index, int role) const override;

			bool isLocked(const JDObject& obj) const
			{
				return m_lockedObjects.find(obj->getShallowObjectID()) != m_lockedObjects.end();
			}
			JDManager::LockedObject getLockedObjectData(const JDObject& obj) const
			{
				auto it = m_lockedObjects.find(obj->getShallowObjectID());
				if (it != m_lockedObjects.end())
				{
					return it->second;
				}
				return JDManager::LockedObject();
			}
			

			void clearFilters();
			void setFilter(std::shared_ptr<Filter> filter);
			void setFilter(const std::vector<std::shared_ptr<Filter>>& filters);
			void addFilter(std::shared_ptr<Filter> filter);
			void removeFilter(std::shared_ptr<Filter> filter);
			void setSorter(std::shared_ptr<Sorter> sorter);
			
		signals:
			void objectClicked(JDObject obj);

		public slots:
			void onFilterChanged();
			void onSort();

		private slots:
			void onManagerDestroyed();

			void onObjectAdded(std::vector<JDObject> objs);
			void onObjectRemoved(std::vector<JDObject> objs);
			//void onObjectLocked(JDObject obj);
			//void onObjectUnlocked(JDObject obj);
			void onLockedObjectsChanged();


			
			

		private:
			//void updateObjectItemLockState(size_t index, bool isLocked);
			
			void addObject_iternal(const JDObject& obj);
			void removeObject_internal(const JDObject& obj);
			size_t findObjectIndex(const JDObject& object) const;
			bool filterObject(const JDObject& obj) const;
			void filterObjects(const std::vector<JDObject>& objs, std::vector<JDObject> & objsOut) const;

			struct JDObjectItem
			{
				JDObject object;
				QStandardItem* item = nullptr;
			};
			std::vector<JDObjectItem> m_objects;
			std::unordered_map<JDObjectID::IDType, JDManager::LockedObject> m_lockedObjects;
			JDManager* m_manager = nullptr;

			std::vector<std::shared_ptr<Filter>> m_filters;
			std::shared_ptr<Sorter> m_sorter;
		};
	}
}