#pragma once

#include "JsonDatabase_base.h"
#include "object/JDObjectInterface.h"
#include "manager/JDManager.h"

#include <QWidget>
#include <QListView>
#include <QStandardItemModel>

namespace JsonDatabase
{
	namespace UI
	{
		class JSON_DATABASE_EXPORT JDObjectListWidget : public QWidget
		{
			Q_OBJECT
			public:
			JDObjectListWidget(JDManager* manager, QWidget* parent = nullptr);
			~JDObjectListWidget();

			//void setObjects(const std::vector<JDObject>& objects);
			void update();

			private slots:
			void onObjectLocked(JDObject obj);
			void onObjectUnlocked(JDObject obj);

			private:
			void setupUI();
			void updateUI();
			size_t findObjectIndex(const JDObject& object) const;

			QListView* m_objectListWidget;
			QStandardItemModel* m_model;

			struct JDObjectItem
			{
				JDObject object;
				QStandardItem* item = nullptr;
			};
			std::vector<JDObjectItem> m_objects;
			JDManager* m_manager = nullptr;
			//std::unordered_map<JDObject, JDObjectItem> m_objectItems;

			static const QString s_iconLock;
			static const QString s_iconUnlock;
		};
	}
}