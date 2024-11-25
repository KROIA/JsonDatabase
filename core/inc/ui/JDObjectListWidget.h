#pragma once

#include "JsonDatabase_base.h"
#include "object/JDObjectInterface.h"
#include "manager/JDManager.h"
#include "utilities/JDObjectItemModel.h"
#include "utilities/JDObjectModelDelegate.h"

#include <QWidget>
#include <QListView>
#include <QVBoxLayout>

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

			void setEnableSearchField(bool enable);
			bool isSearchFieldEnabled() const;

			signals:
			void objectClicked(JDObject obj);
			void objectDoubleClicked(JDObject obj);

			private slots:
			void onItemClicked(const QModelIndex& index);
			void onItemDoubleClicked(const QModelIndex& index);


			void clearFilters();
			void setFilter(std::shared_ptr<Utilities::JDObjectItemModel::Filter> filter);
			void setFilter(const std::vector<std::shared_ptr<Utilities::JDObjectItemModel::Filter>>& filters);
			void addFilter(std::shared_ptr<Utilities::JDObjectItemModel::Filter> filter);
			void setSorter(std::shared_ptr<Utilities::JDObjectItemModel::Sorter> sorter);
			
			public slots:
			void onFilterChanged();
			void onSort();

			private slots:
			void onLineEditChanged(const QString& text);
			private:
			
			void setupUI();
			void updateUI();

			QListView* m_objectListWidget;
			QLineEdit* m_searchBox;
			QFrame* m_searchFrame;
			QVBoxLayout* m_mainLayout;
			Utilities::JDObjectItemModel* m_model;
			Utilities::JDObjectModelDelegate* m_delegate;

			JDManager* m_manager = nullptr;

			class SearchFilter : public Utilities::JDObjectItemModel::Filter
			{
				public:
				SearchFilter()
					: Utilities::JDObjectItemModel::Filter()
					, m_search("")
				{}
				virtual ~SearchFilter() = default;

				void setSearchFilter(const std::string& search)
				{
					m_search = search;
				}
				bool filter(const JDObject& obj) const override
				{
					if (m_search.empty())
						return true;
					return obj->getDisplayName().find(m_search) != std::string::npos;
				}

				private:
				std::string m_search;
			};
			std::shared_ptr<SearchFilter> m_searchFilter;
			//std::unordered_map<JDObject, JDObjectItem> m_objectItems;

		
		};
	}
}