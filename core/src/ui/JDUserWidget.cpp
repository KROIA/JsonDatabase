#include "ui/JDUserWidget.h"
#include "utilities/ResourceManager.h"

#include <QGridLayout>

namespace JsonDatabase
{
	namespace UI
	{
		QString JDUserWidget::s_defaultDateTimeFormat = "dd.MM.yyyy hh:mm:ss";
		std::vector<JDUserWidget*> JDUserWidget::s_instances;


		JDUserWidget::JDUserWidget(QWidget* parent)
			: QWidget(parent)
		{
			s_instances.push_back(this);
			setupUI();
		}
		JDUserWidget::JDUserWidget(const Utilities::JDUser& user, QWidget* parent)
			: QWidget(parent)
			, m_user(user)
		{
			s_instances.push_back(this);
			setupUI();
			updateUI();
		}
		JDUserWidget::~JDUserWidget()
		{
			auto it = std::find(s_instances.begin(), s_instances.end(), this);
			if (it != s_instances.end())
				s_instances.erase(it);
		}

		void JDUserWidget::setupUI()
		{
			// Create a grid layout and set it as the layout for this widget
			QVBoxLayout* backgroundLayout = new QVBoxLayout(this);
			setLayout(backgroundLayout);
			backgroundLayout->setContentsMargins(0, 0, 0, 0);

			m_frame = new QFrame(this);
			QGridLayout* layout = new QGridLayout(m_frame);
			m_frame->setLayout(layout);
			m_frame->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
			m_frame->setFrameShape(QFrame::Box);
			m_frame->setLineWidth(1);
			layout->setContentsMargins(3, 3, 3, 3);
			backgroundLayout->addWidget(m_frame);

			

			// Add rows with 2 columns for the user data
			QLabel* sessionIDLabel = new QLabel(this);
			QLabel* nameLabel = new QLabel(this);
			QLabel* loginTimeLabel = new QLabel(this);

			if (Utilities::ResourceManager::hasIcon(Utilities::ResourceManager::Icon::tag))
				sessionIDLabel->setPixmap(Utilities::ResourceManager::getIcon(Utilities::ResourceManager::Icon::tag).pixmap(16, 16));
			else
				sessionIDLabel->setText("Session ID: ");

			if (Utilities::ResourceManager::hasIcon(Utilities::ResourceManager::Icon::user))
				nameLabel->setPixmap(Utilities::ResourceManager::getIcon(Utilities::ResourceManager::Icon::user).pixmap(16, 16));
			else
				nameLabel->setText("Name: ");

			if (Utilities::ResourceManager::hasIcon(Utilities::ResourceManager::Icon::clock))
				loginTimeLabel->setPixmap(Utilities::ResourceManager::getIcon(Utilities::ResourceManager::Icon::clock).pixmap(16, 16));
			else
				loginTimeLabel->setText("Login Time: ");

			m_sessionIDLabel = new QLabel(this);
			m_nameLabel = new QLabel(this);
			m_loginTimeDateLabel = new QLabel(this);

			// enable text selection on the labels
			m_sessionIDLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
			m_nameLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
			m_loginTimeDateLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);


			sessionIDLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
			nameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
			loginTimeLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
			m_sessionIDLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
			m_nameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
			m_loginTimeDateLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

			layout->setColumnStretch(0, 0);
			layout->setColumnStretch(1, 1);

			
			layout->addWidget(nameLabel, 0, 0);
			layout->addWidget(m_nameLabel, 0, 1);
			layout->addWidget(sessionIDLabel, 1, 0);
			layout->addWidget(m_sessionIDLabel, 1, 1);
			layout->addWidget(loginTimeLabel, 2, 0);
			layout->addWidget(m_loginTimeDateLabel, 2, 1);

			// Set tooltips
			nameLabel->setToolTip("The name of the user.");
			m_nameLabel->setToolTip(nameLabel->toolTip());
			sessionIDLabel->setToolTip("The session ID of the user.");
			m_sessionIDLabel->setToolTip(sessionIDLabel->toolTip());
			loginTimeLabel->setToolTip("The time and date when the user logged in.");
			m_loginTimeDateLabel->setToolTip(loginTimeLabel->toolTip());

		}
		void JDUserWidget::updateUI()
		{
			m_sessionIDLabel->setText(QString::fromStdString(m_user.getSessionID()));
			m_nameLabel->setText(QString::fromStdString(m_user.getName()));
			QDateTime loginDateTime(m_user.getLoginDate(), m_user.getLoginTime());
			m_loginTimeDateLabel->setText(loginDateTime.toString(s_defaultDateTimeFormat));
		}

		void JDUserWidget::setUser(const Utilities::JDUser& user)
		{
			m_user = user;
			updateUI();
		}

		void JDUserWidget::setDefaultDateTimeFormat(const QString& format)
		{
			s_defaultDateTimeFormat = format;

			for (auto w : s_instances)
				w->updateUI();
		}
	}
}