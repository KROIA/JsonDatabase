#pragma once

#include "JsonDatabase_base.h"
#include "object/JDObjectInterface.h"
#include "manager/JDManager.h"
#include "utilities/JDObjectItemModel.h"
#include "utilities/JDObjectModelDelegate.h"

#include <QWidget>
#include <QListView>

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

			signals:
			void objectClicked(JDObject obj);
			void objectDoubleClicked(JDObject obj);

			private slots:
			void onItemClicked(const QModelIndex& index);
			void onItemDoubleClicked(const QModelIndex& index);


			private:
			
			void setupUI();
			void updateUI();

			QListView* m_objectListWidget;
			Utilities::JDObjectItemModel* m_model;
			Utilities::JDObjectModelDelegate* m_delegate;

			JDManager* m_manager = nullptr;
			//std::unordered_map<JDObject, JDObjectItem> m_objectItems;

		
		};
	}
}