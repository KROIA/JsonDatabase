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
			JDObjectItemModel(JDManager *manager, QObject* parent = nullptr);
			~JDObjectItemModel();

			JDObject getObject(const QModelIndex& index) const;
			QModelIndex getIndex(const JDObject& obj) const;

			// Override data function
			QVariant data(const QModelIndex& index, int role) const override;

		signals:
			void objectClicked(JDObject obj);

		private slots:
			void onManagerDestroyed();

			void onObjectAdded(JDObject obj);
			void onObjectRemoved(JDObject obj);
			void onObjectLocked(JDObject obj);
			void onObjectUnlocked(JDObject obj);
			void onLockedObjectsChanged();

			
			

		private:
			void updateObjectItemLockState(size_t index, bool isLocked);

			size_t findObjectIndex(const JDObject& object) const;

			struct JDObjectItem
			{
				JDObject object;
				QStandardItem* item = nullptr;
			};
			std::vector<JDObjectItem> m_objects;
			std::unordered_map<JDObjectID::IDType, JDManager::LockedObject> m_lockedObjects;
			JDManager* m_manager = nullptr;

			static const QString s_iconLock;
			static const QString s_iconUnlock;
		};
	}
}