#include "ui/JDUserWidget.h"
#include "utilities/ResourceManager.h"

#include <QGridLayout>

namespace JsonDatabase
{
	namespace UI
	{
		QString JDUserWidget::s_defaultDateTimeFormat = "dd.MM.yyyy hh:mm:ss";
		std::vector<JDUserWidget*> JDUserWidget::s_instances;

		const QString JDUserWidget::s_iconSessionID = "tag.png";
		const QString JDUserWidget::s_iconName = "user.png";
		const QString JDUserWidget::s_iconLoginTime = "clock.png";


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
			QGridLayout* layout = new QGridLayout(this);
			setLayout(layout);

			// Add rows with 2 columns for the user data
			QLabel* sessionIDLabel = new QLabel(this);
			QLabel* nameLabel = new QLabel(this);
			QLabel* loginTimeLabel = new QLabel(this);

			if (Utilities::ResourceManager::hasIcon(s_iconSessionID))
				sessionIDLabel->setPixmap(Utilities::ResourceManager::getIcon(s_iconSessionID).pixmap(16, 16));
			else
				sessionIDLabel->setText("Session ID: ");

			if (Utilities::ResourceManager::hasIcon(s_iconName))
				nameLabel->setPixmap(Utilities::ResourceManager::getIcon(s_iconName).pixmap(16, 16));
			else
				nameLabel->setText("Name: ");

			if (Utilities::ResourceManager::hasIcon(s_iconLoginTime))
				loginTimeLabel->setPixmap(Utilities::ResourceManager::getIcon(s_iconLoginTime).pixmap(16, 16));
			else
				loginTimeLabel->setText("Login Time: ");

			m_sessionIDLabel = new QLabel(this);
			m_nameLabel = new QLabel(this);
			m_loginTimeLabel = new QLabel(this);

			// enable text selection on the labels
			m_sessionIDLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
			m_nameLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
			m_loginTimeLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);


			sessionIDLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
			nameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
			loginTimeLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
			m_sessionIDLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
			m_nameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
			m_loginTimeLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

			layout->setColumnStretch(0, 0);
			layout->setColumnStretch(1, 1);

			layout->addWidget(sessionIDLabel, 0, 0);
			layout->addWidget(m_sessionIDLabel, 0, 1);
			layout->addWidget(nameLabel, 1, 0);
			layout->addWidget(m_nameLabel, 1, 1);
			layout->addWidget(loginTimeLabel, 2, 0);
			layout->addWidget(m_loginTimeLabel, 2, 1);
		}
		void JDUserWidget::updateUI()
		{
			m_sessionIDLabel->setText(QString::fromStdString(m_user.getSessionID()));
			m_nameLabel->setText(QString::fromStdString(m_user.getName()));
			QDateTime loginDateTime(m_user.getLoginDate(), m_user.getLoginTime());
			m_loginTimeLabel->setText(loginDateTime.toString(s_defaultDateTimeFormat));
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