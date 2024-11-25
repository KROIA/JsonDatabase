#include "ui/JDObjectListWidget.h"
#include "utilities/ResourceManager.h"



#include <QLineEdit>
#include <QLabel>
#include <QFrame>

namespace JsonDatabase
{
	namespace UI
	{
		

		JDObjectListWidget::JDObjectListWidget(JDManager* manager, QWidget* parent)
			: QWidget(parent)
			, m_manager(manager)
		{
			
			m_searchFilter = std::make_shared<SearchFilter>();
			setupUI();
			//setEnableSearchField(false);
		}
		JDObjectListWidget::~JDObjectListWidget()
		{

		}

		void JDObjectListWidget::setEnableSearchField(bool enable)
		{
			if (enable)
			{
				m_searchFrame->show();
				m_model->addFilter(m_searchFilter);
			}
			else
			{
				m_searchFrame->hide();
				m_model->removeFilter(m_searchFilter);
			}
		}
		bool JDObjectListWidget::isSearchFieldEnabled() const
		{
			return m_searchBox->isEnabled();
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

		void JDObjectListWidget::clearFilters()
		{
			m_model->clearFilters();
		}
		void JDObjectListWidget::setFilter(std::shared_ptr<Utilities::JDObjectItemModel::Filter> filter)
		{
			m_model->setFilter(filter);
		}
		void JDObjectListWidget::setFilter(const std::vector<std::shared_ptr<Utilities::JDObjectItemModel::Filter>>& filters)
		{
			m_model->setFilter(filters);
		}
		void JDObjectListWidget::addFilter(std::shared_ptr<Utilities::JDObjectItemModel::Filter> filter)
		{
			m_model->addFilter(filter);
		}
		void JDObjectListWidget::setSorter(std::shared_ptr<Utilities::JDObjectItemModel::Sorter> sorter)
		{
			m_model->setSorter(sorter);
		}
		void JDObjectListWidget::onFilterChanged()
		{
			m_model->onFilterChanged();
		}
		void JDObjectListWidget::onSort()
		{
			m_model->onSort();
		}
		
		void JDObjectListWidget::setupUI()
		{
			m_mainLayout = new QVBoxLayout(this);
			setLayout(m_mainLayout);

			m_searchFrame = new QFrame(this);
			QHBoxLayout* searchLayout = new QHBoxLayout();
			m_searchFrame->setLayout(searchLayout);
			m_searchBox = new QLineEdit(m_searchFrame);
			connect(m_searchBox, &QLineEdit::textChanged, this, &JDObjectListWidget::onLineEditChanged);
			// add search icon to the search box
			QLabel* searchIcon = new QLabel(m_searchFrame);
			searchIcon->setPixmap(Utilities::ResourceManager::getIcon(Utilities::ResourceManager::Icon::search).pixmap(16, 16));
			searchIcon->setAlignment(Qt::AlignCenter);
			m_searchBox->setClearButtonEnabled(true);
			m_searchBox->setPlaceholderText("Search");
			searchLayout->addWidget(searchIcon);
			searchLayout->addWidget(m_searchBox);
			m_mainLayout->addWidget(m_searchFrame);

			m_objectListWidget = new QListView(this);
			m_objectListWidget->setIconSize(QSize(48, 48)); // Set the icon size if needed
			m_mainLayout->addWidget(m_objectListWidget);

			

			m_model = new Utilities::JDObjectItemModel(m_manager, this);
			m_model->setFilter(m_searchFilter);
			m_delegate = new Utilities::JDObjectModelDelegate(m_model);

			
			
			
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

		void JDObjectListWidget::onLineEditChanged(const QString& text)
		{
			m_searchFilter->setSearchFilter(text.toStdString());
			onFilterChanged();
		}
	}
}