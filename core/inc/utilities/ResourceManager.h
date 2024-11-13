#pragma once

#include "JsonDatabase_base.h"
#include <QMap>
#include <QIcon>

namespace JsonDatabase
{
	namespace Utilities
	{
		class JSON_DATABASE_EXPORT ResourceManager
		{
			ResourceManager();
			static ResourceManager& getInstance();
			public:

			static bool hasIcon(const QString& name);
			static const QIcon& getIcon(const QString& name);

			private:
			void loadIcons();

			QMap<QString, QIcon> m_icons;
		};
	}
}