#pragma once

#include "JsonDatabase_base.h"
#include "utilities/JDUser.h"

#include <QWidget>
#include <QLabel>
#include <QFrame>

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

			QFrame* m_frame;
			QLabel* m_sessionIDLabel;
			QLabel* m_nameLabel;
			QLabel* m_loginTimeDateLabel;

			static QString s_defaultDateTimeFormat;
			static std::vector<JDUserWidget*> s_instances;

	
			
		};
	}
}