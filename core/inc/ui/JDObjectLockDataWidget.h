#pragma once

#include "JsonDatabase_base.h"
#include "manager/JDObjectLocker.h"
#include "ui/JDUserWidget.h"
#include <QLabel>

namespace JsonDatabase
{
	namespace UI
	{
		class JSON_DATABASE_EXPORT JDObjectLockDataWidget : public QWidget
		{
			Q_OBJECT
			public:
			JDObjectLockDataWidget(QWidget* parent = nullptr);
			JDObjectLockDataWidget(const Internal::JDObjectLocker::LockData &data, QWidget* parent = nullptr);
			~JDObjectLockDataWidget();

			static void setDefaultDateTimeFormat(const QString& format);
			static const QString& getDefaultDateTimeFormat() { return s_defaultDateTimeFormat; }

			void setLockData(const Internal::JDObjectLocker::LockData &data);
			
			
			private:
			void setupUI();
			void updateUI();

			Internal::JDObjectLocker::LockData m_data;

			JDUserWidget* m_userWidget;
			QLabel* m_objectIDLabel;
			QLabel* m_lockDateTimeLabel;

			static QString s_defaultDateTimeFormat;
		};
	}
}