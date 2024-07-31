#pragma once

#include "JsonDatabase_base.h"
#include "JsonDatabase_Declaration.h"
#include "utilities/filesystem/LockedFileAccessor.h"
#include "utilities/JDSerializable.h"
#include "json/JsonValue.h"

#include "Logger.h"

namespace JsonDatabase
{
	namespace Utilities
	{
		class JSON_DATABASE_EXPORT AbstractRegistry
		{
		public:
			AbstractRegistry();
			virtual ~AbstractRegistry();

			void setParentLogger(Log::LogObject* parentLogger, const std::string &registryName);

			void setDatabasePath(const std::string& path);
			void setName(const std::string& name); // creates a subfolder in the database folder
			void setFileEnding(const std::string& fileEnding);

			const std::string &getDatabasePath() const;
			const std::string &getName() const;
			const std::string &getFileEnding() const;

			void createFiles();


		protected:

			// Manages the closing of the registry file.
			class JSON_DATABASE_EXPORT AutoClose
			{
			public:
				AutoClose(const AbstractRegistry* registry);
				~AutoClose();
			private:
				const AbstractRegistry* m_registry;
			};

			class JSON_DATABASE_EXPORT LockEntryObject : public JDSerializable
			{
			public:
				LockEntryObject(const std::string &key);
				virtual ~LockEntryObject();

				const std::string& getKey() const;
				static std::string getKey(const JsonObject& obj);

				bool load(const JsonObject& obj) override;
				bool save(JsonObject& obj) const override;
			private:
				std::string m_key;
			};
			

			virtual void onCreateFiles() = 0;

			virtual void onDatabasePathChangeStart(const std::string& newPath) = 0;
			virtual void onDatabasePathChangeEnd() = 0;
			virtual void onNameChange(const std::string& newName) = 0;

			// Returns the path in which the registration file is located
			std::string getPath() const;

			// Returns the path to the registration file
			std::string getRegistrationFilePath() const;

			// Returns the name of the registration file "registry"
			std::string getRegistrationFileName() const;

			// Returns the full name to the registration file "registry.json"
			std::string getRegistrationFullFileName() const;

			std::string getLocksPath() const;
			std::string getLockFilePath(const std::string& key) const;

			bool openRegistryFile() const;
			bool openRegistryFile(unsigned int timeoutMillis) const;

			bool isRegistryFileOpen() const;

			bool closeRegistryFile() const;

			// Returns the amount of objects that are saved successfully
			int addObjects(const std::vector<std::shared_ptr<LockEntryObject>> & objects);
			int removeObjects(const std::vector<std::shared_ptr<LockEntryObject>> & objects);
			int removeObjects(const std::vector<std::string> & keys);
			bool isObjectActive(const std::string& key) const;



			template <typename T>
			bool readObjects(std::vector<std::shared_ptr<T>> & objects) const;


			
			bool removeAllSelfOwnedObjects();
			int removeInactiveObjects() const;
			bool lockExists(const std::string& key) const;
			bool isSelfOwned(const std::string& key) const;

			std::vector<std::string> getLockNames() const;
			std::vector<std::string> getSelfOwnedLockNames() const;
			std::vector<std::string> getNotSelfOwnedLockNames() const;
			unsigned int getLockCount() const;
			unsigned int getSelfLockCount() const;
			unsigned int getNotSelfLockCount() const;


			Log::LogObject* m_logger = nullptr;

		private:
			bool createSelfOwnedLock(const std::string& key);
			bool removeSelfOwnedLock(const std::string& key);
			int saveObjects_internal(const JsonArray& jsons) const;
			bool readObjects_internal(JsonArray& jsons) const;
			bool readObjects_internal(const JsonArray& jsons, std::vector<JDSerializable*>& objects) const;

			

			
			std::string m_databasePath;
			std::string m_registrationName;
			std::string m_registrationFileEnding;

			std::string m_fileLocksPath;
			//const std::string m_fileLocksPrefix = "lock_";

			mutable Internal::LockedFileAccessor* m_registryFile;
			std::unordered_map<std::string, Internal::FileLock*> m_fileLocks;

			bool m_nameSet;
			bool m_databasePathSet;
		};

		template <typename T>
		bool AbstractRegistry::readObjects(std::vector<std::shared_ptr<T>>& objects) const
		{
			if (!isRegistryFileOpen())
				return false;
			JsonArray jsons;
			bool success = readObjects_internal(jsons);
			if(success)
			{
				objects.clear();
				objects.reserve(jsons.size());
				for(auto& json : jsons)
				{
					JsonObject& jsonData = json.get<JsonObject>();
					std::string loadedKey = LockEntryObject::getKey(jsonData);
					std::shared_ptr<T> object = std::make_shared<T>(loadedKey);
					if (object->load(jsonData))
						objects.push_back(object);
					else
						success = false;
				}
				
			}
			return success;
		}
	}
}
