#include "utilities/ResourceManager.h"

#include <QDirIterator>
#include <QApplication>


static void loadJDResources()
{
	Q_INIT_RESOURCE(JDIcons);
}

namespace JsonDatabase
{
	namespace Utilities
	{
		ResourceManager::ResourceManager()
		{
			// Load QT resourse file
			loadJDResources();
			loadIcons();
		}

		ResourceManager& ResourceManager::getInstance()
		{
			static ResourceManager instance;
			return instance;
		}

		bool ResourceManager::hasIcon(const QString& name)
		{
			return getInstance().m_icons.find(name) != getInstance().m_icons.end();
		}
		bool ResourceManager::hasIcon(Icon icon)
		{
			if (static_cast<int>(icon) >= getInstance().m_iconsVector.size())
				return false;
			return getInstance().m_iconsVector[static_cast<int>(icon)].isNull();
		}
		const QIcon& ResourceManager::getIcon(const QString& name)
		{
			return getInstance().getIcon_internal(name);
		}
		const QIcon& ResourceManager::getIcon_internal(const QString& name) const
		{
			const auto& it = m_icons.find(name);
			if (it == m_icons.end())
			{
				static QIcon emptyIcon;
				qWarning() << "ResourceManager::getIcon: Icon not found: " << name;
				return emptyIcon;
			}
			return it.value();
		}


		const QIcon& ResourceManager::getIcon(Icon icon)
		{
			return getInstance().getIcon_internal(icon);
		}

		
		const QIcon& ResourceManager::getIcon_internal(Icon icon) const
		{
			return m_iconsVector[static_cast<int>(icon)];
		}


		void ResourceManager::loadIcons()
		{
			// load all files from the resourse file
			QDirIterator it(":/JDIcons", QDirIterator::Subdirectories);
			QMap<QString, QIcon> icons;
			while (it.hasNext())
			{
				it.next();
				if (it.fileInfo().isFile())
				{
					QIcon icon(it.filePath());
					icons[it.fileName()] = icon;
				}
			}
			m_icons = icons;

			m_iconsVector.resize(static_cast<int>(Icon::__count));
			m_iconsVector[static_cast<int>(Icon::accept)] = getIcon_internal("accept.png");
			m_iconsVector[static_cast<int>(Icon::clock)] = getIcon_internal("clock.png");
			m_iconsVector[static_cast<int>(Icon::lock)] = getIcon_internal("lock.png");
			m_iconsVector[static_cast<int>(Icon::tag)] = getIcon_internal("tag.png");
			m_iconsVector[static_cast<int>(Icon::unlock)] = getIcon_internal("unlock.png");
			m_iconsVector[static_cast<int>(Icon::user)] = getIcon_internal("user.png");
			// static assert when not all enum values are covered
			static_assert(static_cast<int>(Icon::__count) == 6, "ResourceManager::loadIcons: Not all icons are covered");
		}
	}
}