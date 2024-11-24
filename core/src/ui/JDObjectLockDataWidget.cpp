#include "ui/JDObjectLockDataWidget.h"
#include <QGridLayout>
#include "utilities/ResourceManager.h"

namespace JsonDatabase
{
	namespace UI
	{

		QString JDObjectLockDataWidget::s_defaultDateTimeFormat = "dd.MM.yyyy hh:mm:ss";

		JDObjectLockDataWidget::JDObjectLockDataWidget(QWidget* parent)
			: QWidget(parent)
		{
			setupUI();
		}
		JDObjectLockDataWidget::JDObjectLockDataWidget(const Internal::JDObjectLocker::LockData &data, QWidget* parent)
			: QWidget(parent)
			, m_data(data)
		{
			setupUI();
			updateUI();
		}
		JDObjectLockDataWidget::~JDObjectLockDataWidget()
		{

		}

		void JDObjectLockDataWidget::setLockData(const Internal::JDObjectLocker::LockData& data)
		{
			m_data = data;
			updateUI();
		}

		void JDObjectLockDataWidget::setupUI()
		{
			QGridLayout* layout = new QGridLayout(this);
			setLayout(layout);

			
			//QFrame* userFrame = new QFrame(this);
			m_userWidget = new JDUserWidget(this);
			/*QVBoxLayout* userLayout = new QVBoxLayout(userFrame);
			userFrame->setLayout(userLayout);
			userFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
			userFrame->setFrameShape(QFrame::Box);
			userFrame->setLineWidth(1);
			userLayout->setContentsMargins(3, 3, 3, 3);
			userLayout->addWidget(m_userWidget);
			layout->addWidget(userFrame, 0, 0, 1, 2);*/
			layout->addWidget(m_userWidget, 0, 0, 1, 2);

			QLabel* idLabel = new QLabel(this);
			if (Utilities::ResourceManager::hasIcon(Utilities::ResourceManager::Icon::tag))
				idLabel->setPixmap(Utilities::ResourceManager::getIcon(Utilities::ResourceManager::Icon::tag).pixmap(16, 16));
			else
				idLabel->setText("Object ID: ");
			layout->addWidget(idLabel, 1, 0);
			m_objectIDLabel = new QLabel(this);
			layout->addWidget(m_objectIDLabel, 1, 1);

			QLabel* timeLabel = new QLabel(this);
			if (Utilities::ResourceManager::hasIcon(Utilities::ResourceManager::Icon::clock))
				timeLabel->setPixmap(Utilities::ResourceManager::getIcon(Utilities::ResourceManager::Icon::clock).pixmap(16, 16));
			else
				timeLabel->setText("Lock Date and Time: ");
			layout->addWidget(timeLabel, 2, 0);
			m_lockDateTimeLabel = new QLabel(this);
			layout->addWidget(m_lockDateTimeLabel, 2, 1);

			// enable text selection on the labels
			m_lockDateTimeLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
			m_objectIDLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
	
			m_objectIDLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
			idLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
			m_lockDateTimeLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
			timeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

			// Set tooltips
			m_userWidget->setToolTip("The user that locked the object");
			
			m_objectIDLabel->setToolTip("The ID of the object that is locked");
			idLabel->setToolTip(m_objectIDLabel->toolTip());

			m_lockDateTimeLabel->setToolTip("The date when the object was locked");
			timeLabel->setToolTip(m_lockDateTimeLabel->toolTip());

			// set column stretch
			layout->setColumnStretch(0, 0);
			layout->setColumnStretch(1, 1);

		}

		void JDObjectLockDataWidget::updateUI()
		{
			m_userWidget->setUser(m_data.user);
			m_objectIDLabel->setText(JDObjectID::toQString(m_data.objectID));
			QDateTime lockDateTime = QDateTime(m_data.lockDate, m_data.lockTime);
			m_lockDateTimeLabel->setText(lockDateTime.toString(s_defaultDateTimeFormat));
		}
	}
}
