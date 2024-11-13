#pragma once

#include "JsonDatabase_base.h"
#include "utilities/JDUser.h"

#include <QWidget>
#include <QLabel>
#include <QScrollArea>
#include <QSpacerItem>

namespace JsonDatabase
{
	namespace UI
	{
		class JDUserWidget;
		class JSON_DATABASE_EXPORT JDUserListWidget : public QWidget
		{
			Q_OBJECT
			public:
			JDUserListWidget(QWidget* parent = nullptr);
			JDUserListWidget(const std::vector<Utilities::JDUser>& users, QWidget* parent = nullptr);
			~JDUserListWidget();

			void setUsers(const std::vector<Utilities::JDUser>& users);
			const std::vector<Utilities::JDUser>& getUsers() const { return m_users; }

			void addUser(const Utilities::JDUser& user);
			void removeUser(const Utilities::JDUser& user);

			private:
			void setupUI();
			void updateUI();
			QScrollArea* m_userListWidget;
			QWidget* m_userListWidgetContent;
			QSpacerItem* m_spacer;
			std::vector<Utilities::JDUser> m_users;
			std::vector<JDUserWidget*> m_userWidgets;
		};
	}
}