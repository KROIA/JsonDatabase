#pragma once

#include "JsonDatabase_base.h"
#include "utilities/JDUser.h"

#include <QWidget>
#include <QLabel>

namespace JsonDatabase
{
	namespace UI
	{
		class JSON_DATABASE_EXPORT JDUserWidget : public QWidget
		{
			Q_OBJECT
			public:
			JDUserWidget(QWidget* parent = nullptr);
			JDUserWidget(const Utilities::JDUser& user, QWidget* parent = nullptr);
			~JDUserWidget();

			void setUser(const Utilities::JDUser& user);
			const Utilities::JDUser& getUser() const { return m_user; }

			static void setDefaultDateTimeFormat(const QString& format);
			static const QString& getDefaultDateTimeFormat() { return s_defaultDateTimeFormat; }

			static std::vector<JDUserWidget*> getInstances() { return s_instances; }

			private:
			void setupUI();
			void updateUI();
			Utilities::JDUser m_user;

			QLabel* m_sessionIDLabel;
			QLabel* m_nameLabel;
			QLabel* m_loginTimeLabel;

			static QString s_defaultDateTimeFormat;
			static std::vector<JDUserWidget*> s_instances;

			static const QString s_iconSessionID;
			static const QString s_iconName;
			static const QString s_iconLoginTime;
			
		};
	}
}