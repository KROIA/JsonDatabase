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
		const QIcon& ResourceManager::getIcon(const QString& name)
		{
			ResourceManager& instance = getInstance();
			if (instance.m_icons.find(name) == instance.m_icons.end())
			{
				static QIcon emptyIcon;
				qWarning() << "ResourceManager::getIcon: Icon not found: " << name;
				return emptyIcon;
			}
			return instance.m_icons[name];
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
		}
	}
}