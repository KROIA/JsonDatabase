#include "ui/JDUserListWidget.h"
#include "ui/JDUserWidget.h"

#include <QVBoxLayout>

namespace JsonDatabase
{
	namespace UI
	{
		JDUserListWidget::JDUserListWidget(QWidget* parent)
			: QWidget(parent)
		{
			setupUI();
		}
		JDUserListWidget::JDUserListWidget(const std::vector<Utilities::JDUser>& users, QWidget* parent)
			: QWidget(parent)
			, m_users(users)
		{
			setupUI();
			updateUI();
		}
		JDUserListWidget::~JDUserListWidget()
		{
			m_userWidgets.clear();
		}


		void JDUserListWidget::setUsers(const std::vector<Utilities::JDUser>& users)
		{
			//m_users = users;
			//updateUI();

			// remove users that are not in the new list
			for (size_t j = 0; j < m_users.size(); ++j)
			{
				bool found = false;
				for (size_t i = 0; i < users.size(); ++i)
				{
					if (m_users[j].getSessionID() == users[i].getSessionID())
					{
						found = true;
						break;
					}
				}
				if (!found)
				{
					removeUser(m_users[j]);
					--j;
				}
			}

			for (size_t i = 0; i < users.size(); ++i)
			{
				for (size_t j = 0; j < m_users.size(); ++j)
				{
					if (users[i].getSessionID() == m_users[j].getSessionID())
					{
						m_users[j] = users[i];
						m_userWidgets[j]->setUser(users[i]);
						goto nextUser;
					}
				}
				addUser(users[i]);
				nextUser:;
			}
		}

		void JDUserListWidget::addUser(const Utilities::JDUser& user)
		{
			// Check if user is already in the list
			for (const Utilities::JDUser& u : m_users)
			{
				if (u == user)
					return;
			}

			m_users.push_back(user);
			JDUserWidget* userWidget = new JDUserWidget(user, this);
			m_userWidgets.push_back(userWidget);
			QLayout* layout = m_userListWidgetContent->layout();
			layout->removeItem(m_spacer);
			layout->addWidget(userWidget);
			layout->addItem(m_spacer);
		}
		void JDUserListWidget::removeUser(const Utilities::JDUser& user)
		{
			for (size_t i = 0; i < m_users.size(); ++i)
			{
				if (m_users[i].getSessionID() == user.getSessionID())
				{
					m_users.erase(m_users.begin() + i);
					delete m_userWidgets[i];
					m_userWidgets.erase(m_userWidgets.begin() + i);
					return;
				}
			}
		}

		void JDUserListWidget::setupUI()
		{
			m_userListWidget = new QScrollArea(this);
			m_spacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
			m_userListWidgetContent = new QWidget(m_userListWidget);
			//QVBoxLayout* layout = new QVBoxLayout(m_userListWidget);
			QVBoxLayout* layout2 = new QVBoxLayout(this);
			QVBoxLayout* layout3 = new QVBoxLayout(m_userListWidgetContent);

			m_userListWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
			m_userListWidget->setWidgetResizable(true);
			m_userListWidget->setWidget(m_userListWidgetContent);
			
			// Make elements allign on the top
			layout3->setAlignment(Qt::AlignTop);
			//m_userListWidget->setLayout(layout);
			layout3->addItem(m_spacer);

			this->setLayout(layout2);
			layout2->addWidget(m_userListWidget);
		}
		void JDUserListWidget::updateUI()
		{
			for (size_t i = 0; i < m_userWidgets.size(); ++i)
			{
				delete m_userWidgets[i];
			}
			m_userWidgets.clear();

			QLayout* layout = m_userListWidgetContent->layout();
			for (size_t i = 0; i < m_users.size(); ++i)
			{
				JDUserWidget* userWidget = new JDUserWidget(m_users[i], this);
				layout->removeItem(m_spacer);
				layout->addWidget(userWidget);
				layout->addItem(m_spacer);
				m_userWidgets.push_back(userWidget);
			}
		}
	}
}