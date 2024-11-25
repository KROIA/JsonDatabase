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
			enum class Icon
			{
				accept,
				clock,
				lock,
				tag,
				unlock,
				user,
				search,

				__count
			};

			static bool hasIcon(const QString& name);
			static bool hasIcon(Icon icon);
			static const QIcon& getIcon(const QString& name);
			static const QIcon& getIcon(Icon icon);

			private:
			void loadIcons();
			const QIcon& getIcon_internal(const QString& name) const;
			const QIcon& getIcon_internal(Icon icon) const;

			QMap<QString, QIcon> m_icons;
			QVector<QIcon> m_iconsVector;
		};
	}
}