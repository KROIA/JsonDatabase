#include "utilities/filesystem/AbstractRegistry.h"
#include "utilities/JDSerializable.h"
#include <QDir>

namespace JsonDatabase
{
	namespace Utilities
	{
		AbstractRegistry::AbstractRegistry()
			: m_databasePath("")
			, m_registrationName("registry")
			, m_registrationFileEnding(".json")
			, m_fileLocksPath("locks")
			, m_registryFile(nullptr)
			, m_nameSet(false)
			, m_databasePathSet(false)
		{
			
		}
		AbstractRegistry::~AbstractRegistry()
		{
			if (AbstractRegistry::openRegistryFile(100))
			{
				AbstractRegistry::AutoClose autoClose(this);
				removeAllSelfOwnedObjects();
			}
			
			delete m_registryFile;
		}

		void AbstractRegistry::setParentLogger(Log::LogObject* parentLogger, const std::string& registryName)
		{
			if(parentLogger)
				m_logger = new Log::LogObject(*parentLogger, registryName);
		}

		void AbstractRegistry::setDatabasePath(const std::string& path)
		{
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_5);
			if (m_databasePath == path)
				return;
			JsonArray backup;
			JsonArray selfOwnedObjects;
			if (m_databasePathSet)
			{
				if (!isRegistryFileOpen())
				{
					openRegistryFile();
				}

				if (readObjects_internal(backup))
				{
					for (size_t i = 0; i < backup.size(); ++i)
					{
						JsonObject& obj = backup[i].get<JsonObject>();
						std::string key = LockEntryObject::getKey(obj);
						if (m_fileLocks.find(key) != m_fileLocks.end())
						{
							selfOwnedObjects.push_back(backup[i]);
						}
					}
				}

				removeAllSelfOwnedObjects();
				closeRegistryFile();

				onDatabasePathChangeStart(path);
			}

			m_databasePath = path;

			if (m_databasePathSet)
			{
				onDatabasePathChangeEnd();
				createFiles();
				openRegistryFile();

				for (size_t i = 0; i < selfOwnedObjects.size(); ++i)
				{
					JsonObject& obj = selfOwnedObjects[i].get<JsonObject>();
					std::string key = LockEntryObject::getKey(obj);
					createSelfOwnedLock(key);
				}
				readObjects_internal(selfOwnedObjects);
				closeRegistryFile();
			}
			m_databasePathSet = true;
		}
		void AbstractRegistry::setName(const std::string& name)
		{
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_5);
			if(name == m_registrationName)
				return;
			std::string newName = name;
			
			if (newName.size() == 0)
				newName = "registry";
		
			JsonArray backup;
			JsonArray selfOwnedObjects;

			if (m_nameSet)
			{
				if (!isRegistryFileOpen())
				{
					openRegistryFile();
				}


				if (readObjects_internal(backup))
				{
					for (size_t i = 0; i < backup.size(); ++i)
					{
						JsonObject& obj = backup[i].get<JsonObject>();
						std::string key = LockEntryObject::getKey(obj);
						if (m_fileLocks.find(key) != m_fileLocks.end())
						{
							selfOwnedObjects.push_back(backup[i]);
						}
					}
				}

				removeAllSelfOwnedObjects();
				closeRegistryFile();
				onNameChange(newName);
			}
			
			m_registrationName = newName;
			

			if (m_nameSet)
			{
				openRegistryFile();
				createFiles();
				for (size_t i = 0; i < selfOwnedObjects.size(); ++i)
				{
					JsonObject& obj = selfOwnedObjects[i].get<JsonObject>();
					std::string key = LockEntryObject::getKey(obj);
					createSelfOwnedLock(key);
				}
				readObjects_internal(selfOwnedObjects);
				closeRegistryFile();
			}
			m_nameSet = true;
		}
		void AbstractRegistry::setFileEnding(const std::string& fileEnding)
		{
			m_registrationFileEnding = fileEnding;
		}

		const std::string& AbstractRegistry::getDatabasePath() const
		{
			return m_databasePath;
		}
		const std::string& AbstractRegistry::getName() const
		{
			return m_registrationName;
		}
		const std::string& AbstractRegistry::getFileEnding() const
		{
			return m_registrationFileEnding;
		}

		void AbstractRegistry::createFiles()
		{
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_5);
			QString path = QString::fromStdString(getPath());
			QDir dir(path);
			if (!dir.exists())
			{
				QDir dir2;
				
				if (dir2.mkpath(path))
				{
					if (m_logger)
						m_logger->logInfo("Creating directory: " + path.toStdString());
				}
				else
				{
					if (m_logger)
						m_logger->logError("Failed to create directory: " + path.toStdString());
				}
			}
			dir.setPath(QString::fromStdString(getLocksPath()));
			if (!dir.exists())
			{
				QDir dir2;
				if (dir2.mkpath(QString::fromStdString(getLocksPath())))
				{
					if (m_logger)
						m_logger->logInfo("Creating directory: " + getLocksPath());
				}
				else
				{
					if (m_logger)
						m_logger->logError("Failed to create directory: " + getLocksPath());
				}
			}
			std::string registrationFile = getRegistrationFilePath();
			if (!QFile::exists(QString::fromStdString(registrationFile)))
			{
				if (AbstractRegistry::openRegistryFile(100))
				{
					AbstractRegistry::AutoClose autoClose(this);
					saveObjects_internal({});
				}
			}
			onCreateFiles();
		}


		AbstractRegistry::AutoClose::AutoClose(const AbstractRegistry* registry)
			: m_registry(registry)
		{ }
		AbstractRegistry::AutoClose::~AutoClose()
		{
			if(m_registry)
				m_registry->closeRegistryFile();
		}


		AbstractRegistry::LockEntryObject::LockEntryObject(const std::string& key)
			: JDSerializable()
			, m_key(key)
		{

		}
		AbstractRegistry::LockEntryObject::~LockEntryObject()
		{

		}

		const std::string& AbstractRegistry::LockEntryObject::getKey() const
		{
			return m_key;
		}
		std::string AbstractRegistry::LockEntryObject::getKey(const JsonObject& obj)
		{
			if(obj.contains("key"))
			{
				return obj.at("key").get<std::string>();
			}
			return "";
		}
		bool AbstractRegistry::LockEntryObject::load(const JsonObject& obj)
		{
			m_key = getKey(obj);
			if(m_key.size() > 0)
				return true;
			return false;
		}

		bool AbstractRegistry::LockEntryObject::save(JsonObject& obj) const
		{
			obj["key"] = m_key;
			return true;
		}


		std::string AbstractRegistry::getPath() const
		{
			return m_databasePath + "\\" + m_registrationName;
		}
		std::string AbstractRegistry::getRegistrationFilePath() const
		{
			return getPath() + "\\" + m_registrationName + m_registrationFileEnding;
		}
		std::string AbstractRegistry::getRegistrationFileName() const
		{
			return m_registrationName;
		}
		std::string AbstractRegistry::getRegistrationFullFileName() const
		{
			return m_registrationName + m_registrationFileEnding;
		}
		std::string AbstractRegistry::getLocksPath() const
		{
			return getPath() + "\\" + m_fileLocksPath;
		}
		std::string AbstractRegistry::getLockFilePath(const std::string& key) const
		{
			return getLocksPath() + "\\" + key;
		}

		bool AbstractRegistry::openRegistryFile() const
		{
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_5);
			if(isRegistryFileOpen())
				return true;
			
			Internal::LockedFileAccessor *file = new Internal::LockedFileAccessor(getPath(), getRegistrationFileName(), getFileEnding(), m_logger);
			Error err = file->lock(Internal::LockedFileAccessor::AccessMode::readWrite);
			if (err != Error::none)
			{
				if (m_logger)
					m_logger->logError("Failed to open registry file: " + file->getFullFilePath() + " Error: "+ errorToString(err));
				delete file;
				return false;
			}

			m_registryFile = file;
			return true;
		}
		bool AbstractRegistry::openRegistryFile(unsigned int timeoutMillis) const
		{
			//JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_5);
			if (isRegistryFileOpen())
				return true;

			Internal::LockedFileAccessor* file = new Internal::LockedFileAccessor(getPath(), getRegistrationFileName(), getFileEnding(), m_logger);
			Error err = file->lock(Internal::LockedFileAccessor::AccessMode::readWrite, timeoutMillis);
			if (err != Error::none)
			{
				delete file;
				return false;
			}

			m_registryFile = file;
			return true;
		}

		bool AbstractRegistry::isRegistryFileOpen() const
		{
			return m_registryFile != nullptr;
		}

		bool AbstractRegistry::closeRegistryFile() const
		{
			if (!isRegistryFileOpen())
				return true;

			Error err = m_registryFile->unlock();
			delete m_registryFile;
			m_registryFile = nullptr;
			if(err != Error::none)
				return false;
			return true;
		}

		int AbstractRegistry::addObjects(const std::vector<std::shared_ptr<LockEntryObject>>& objects)
		{
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_5);
			if (!isRegistryFileOpen())
				return 0;
			JsonArray jsons;
			readObjects_internal(jsons);
			int added = 0;
			for (auto& obj : objects)
			{
				std::shared_ptr<JsonObject> jsonObj = std::make_shared<JsonObject>();
				if (!createSelfOwnedLock(obj->getKey()))
				{
					// Can't create lock
					continue;
				}
				if (obj->save(*jsonObj))
				{
					++added;
					jsons.push_back(std::move(jsonObj));
				}

			}
			
			if(saveObjects_internal(jsons) != 0)
				return added;
			return 0;
		}
		int AbstractRegistry::removeObjects(const std::vector<std::shared_ptr<LockEntryObject>>& objects)
		{
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_5);
			if (!isRegistryFileOpen())
				return 0;
			
			std::vector<std::string> keys;
			for (auto& obj : objects)
			{
				keys.push_back(obj->getKey());
			}
			return removeObjects(keys);
		}
		int AbstractRegistry::removeObjects(const std::vector<std::string>& keys)
		{
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_5);
			if (!isRegistryFileOpen())
				return 0;
			JsonArray jsons;

			readObjects_internal(jsons);
			int removed = 0;
			for (auto& obj : keys)
			{
				std::string key = obj;
				if (m_fileLocks.find(key) == m_fileLocks.end())
					continue; // This lock is not owned by this registry

				for (size_t i = 0; i < jsons.size(); ++i)
				{
					JsonObject& subObj = jsons[i].get<JsonObject>();
					std::string loadedKey = LockEntryObject::getKey(subObj);
					if (loadedKey == key)
					{
						jsons.erase(jsons.begin() + i);
						removeSelfOwnedLock(key);
						++removed;
						break;
					}
				}
			}
			saveObjects_internal(jsons);
			return removed;
		}
		bool AbstractRegistry::isObjectActive(const std::string& key) const
		{
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_5);
			if (!isRegistryFileOpen())
				return false;
			JsonArray jsons;
			readObjects_internal(jsons);
			for (size_t i = 0; i < jsons.size(); ++i)
			{
				JsonObject& subObj = jsons[i].get<JsonObject>();
				std::string loadedKey = LockEntryObject::getKey(subObj);
				if (loadedKey == key)
				{
					return true;
				}
			}
			return false;
		}
		int AbstractRegistry::saveObjects_internal(const JsonArray& jsons) const
		{
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_6);
			if (!isRegistryFileOpen())
				return 0;
			
			auto err = m_registryFile->writeJsonFile(jsons);
			if(err == Error::none)
				return jsons.size();
			return 0;
		}

		bool AbstractRegistry::readObjects_internal(JsonArray& jsons) const
		{
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_6);
			if (!isRegistryFileOpen())
				return 0;

			auto err = m_registryFile->readJsonFile(jsons);
			if (err != Error::none)
				return false;
			return true;
		}

		bool AbstractRegistry::readObjects_internal(const JsonArray& jsons, std::vector<JDSerializable*>& objects) const
		{
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_6);
			bool success = true;
			std::vector<JDSerializable*> successfully;
			for(size_t i=0; i< objects.size(); ++i)
			{
				const JsonObject& subObj = jsons[i].get<JsonObject>();
				bool s = objects[i]->load(subObj);
				if (s)
				{
					successfully.push_back(objects[i]);
				}
				success &= s;
			}
			objects = successfully;
			return  success;
		}



		bool AbstractRegistry::createSelfOwnedLock(const std::string& name)
		{
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_7);
			if(m_fileLocks.find(name) != m_fileLocks.end())
				return false;

			Internal::FileLock* lock = new Internal::FileLock(getLocksPath(), name, m_logger);
			Error err;
			if (!lock->lock(err))
			{
				delete lock;
				return false;
			}
			m_fileLocks[name] = lock;
			return true;
		}
		bool AbstractRegistry::removeSelfOwnedLock(const std::string& name)
		{
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_7);
			const auto &it = m_fileLocks.find(name);
			if (it == m_fileLocks.end())
				return false;

			Internal::FileLock* lock = it->second;
			m_fileLocks.erase(it);
			Error err;
			lock->unlock(err);
			delete lock;
			return err == Error::none;
		}
		bool AbstractRegistry::removeAllSelfOwnedObjects()
		{
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_7);
			if (!isRegistryFileOpen())
				return 0;
			JsonArray jsons;
			JsonArray jsonsOut;
			std::vector<std::string> selfOwned = getSelfOwnedLockNames();
			readObjects_internal(jsons);
			jsonsOut = jsons;
			bool success = true;
			for (auto& obj : selfOwned)
			{
				std::string key = obj;
				if (m_fileLocks.find(key) == m_fileLocks.end())
					continue; // This lock is not owned by this registry

				for (size_t i = 0; i < jsonsOut.size(); ++i)
				{
					JsonObject& subObj = jsonsOut[i].get<JsonObject>();
					std::string loadedKey = LockEntryObject::getKey(subObj);
					if (loadedKey == key)
					{
						jsonsOut.erase(jsonsOut.begin() + i);
						success &= removeSelfOwnedLock(key);
						break;
					}
				}
			}
			if (jsonsOut.size() != jsons.size())
			{
				saveObjects_internal(jsonsOut);
			}
			m_fileLocks.clear();
			return success;
		}
		int AbstractRegistry::removeInactiveObjects() const
		{
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_7);
			if (!isRegistryFileOpen())
				return 0;
			JsonArray jsons;
			JsonArray jsonsOut;
			std::unordered_map<std::string, JsonObject> objectsFromFile;
			if(!readObjects_internal(jsons))
				return 0;
			

			for (auto& obj : jsons)
			{
				JsonObject& subObj = obj.get<JsonObject>();
				std::string key = LockEntryObject::getKey(subObj);
				objectsFromFile[key] = subObj;
			}


			std::vector<std::string> lockFiles = getLockNames();
			
			int removed = 0;

			JD_REGISTRY_PROFILING_BLOCK("Search inactive locks", JD_COLOR_STAGE_8);
			for (size_t i = 0; i < lockFiles.size(); ++i)
			{
				const auto &it = objectsFromFile.find(lockFiles[i]);
				if (Internal::FileLock::deleteFile(getLocksPath(), lockFiles[i]))
				{
					removed++;
					if (it != objectsFromFile.end())
					{
						objectsFromFile.erase(it);
					}
				}	
				else
				{
					if (it != objectsFromFile.end())
					{
						jsonsOut.emplace_back(std::move(it->second));
					}
				}
			}
			JD_REGISTRY_PROFILING_END_BLOCK;
			if (jsonsOut.size() != jsons.size())
			{
				/*for (auto& obj : objectsFromFile)
				{
					jsonsOut.push_back(obj.second);
				}*/
				saveObjects_internal(jsonsOut);
			}
			if(removed > 0 && m_logger)
				m_logger->logInfo("Removed " + std::to_string(removed) + " inactive locks");
				
			return removed;
		}
		bool AbstractRegistry::lockExists(const std::string& name) const
		{
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_6);
			/*/const auto& it = m_fileLocks.find(name);
			if (it == m_fileLocks.end())
				return false;*/

			std::string path = getLocksPath();
			if (Internal::FileLock::fileExists(path, name))
				return true;
			return false;
		}
		bool AbstractRegistry::isSelfOwned(const std::string& key) const
		{
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_6);
			const auto& it = m_fileLocks.find(key);
			if (it != m_fileLocks.end())
				return true;
			return false;
		}

		std::vector<std::string> AbstractRegistry::getLockNames() const
		{
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_6);
			std::vector<std::string> files = Internal::FileLock::getLockFileNamesInDirectory(getLocksPath());
			return files;
		}
		std::vector<std::string> AbstractRegistry::getSelfOwnedLockNames() const
		{
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_6);
			std::vector<std::string> names;
			for (auto& lock : m_fileLocks)
			{
				names.push_back(lock.first);
			}
			return names;
		}
		std::vector<std::string> AbstractRegistry::getNotSelfOwnedLockNames() const
		{
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_6);
			std::vector<std::string> names = getLockNames();
			std::vector<std::string> selfOwned = getSelfOwnedLockNames();

			for (auto& self : selfOwned)
			{
				for (auto it = names.begin(); it != names.end();)
				{
					if (*it == self)
						it = names.erase(it);
					else
						++it;
				}
			}
			return names;
		}
		unsigned int AbstractRegistry::getLockCount() const
		{
			std::vector<std::string> files = Internal::FileLock::getLockFileNamesInDirectory(getLocksPath());
			return files.size();
		}
		unsigned int AbstractRegistry::getSelfLockCount() const
		{
			return m_fileLocks.size();
		}
		unsigned int AbstractRegistry::getNotSelfLockCount() const
		{
			return getLockCount() - getSelfLockCount();
		}
	}
}