#pragma once

#include "JD_base.h"
#include "JDDeclaration.h"
#include "utilities/filesystem/LockedFileAccessor.h"
#include "utilities/JDSerializable.h"

#ifdef JD_USE_QJSON
#include <QJsonObject>
#else
#include <json/JsonValue.h>
#endif

namespace JsonDatabase
{
	namespace Utilities
	{
		class JSONDATABASE_EXPORT AbstractRegistry
		{
		public:
			AbstractRegistry();
			virtual ~AbstractRegistry();


			void setDatabasePath(const std::string& path);
			void setName(const std::string& name); // creates a subfolder in the database folder
			void setFileEnding(const std::string& fileEnding);

			const std::string &getDatabasePath() const;
			const std::string &getName() const;
			const std::string &getFileEnding() const;

			void createFiles();


		protected:

			// Manages the closing of the registry file.
			class JSONDATABASE_EXPORT AutoClose
			{
			public:
				AutoClose(const AbstractRegistry* registry);
				~AutoClose();
			private:
				const AbstractRegistry* m_registry;
			};

			class JSONDATABASE_EXPORT LockEntryObject : public JDSerializable
			{
			public:
				LockEntryObject(const std::string &key);
				virtual ~LockEntryObject();

				const std::string& getKey() const;
#ifdef JD_USE_QJSON
				static std::string getKey(const QJsonObject& obj);
#else
				static std::string getKey(const JsonObject& obj);
#endif

#ifdef JD_USE_QJSON
				bool load(const QJsonObject& obj) override;
				bool save(QJsonObject& obj) const override;
#else
				bool load(const JsonObject& obj) override;
				bool save(JsonObject& obj) const override;
#endif
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

/*#ifdef JD_USE_QJSON
			int saveObjects(const std::vector<QJsonObject>& jsons);
#else
			int saveObjects(const JsonArray& jsons);
#endif*/

			template <typename T>
			bool readObjects(std::vector<std::shared_ptr<T>> & objects) const;
/*
#ifdef JD_USE_QJSON
			bool readObjects(std::vector<QJsonObject> &jsons);
#else
			bool readObjects(JsonArray& jsons);
#endif*/

			
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




		private:
			bool createSelfOwnedLock(const std::string& key);
			bool removeSelfOwnedLock(const std::string& key);
#ifdef JD_USE_QJSON
			int saveObjects_internal(const std::vector<QJsonObject>& jsons) const;
#else
			int saveObjects_internal(const JsonArray& jsons) const;
#endif
#ifdef JD_USE_QJSON
			bool readObjects_internal(std::vector<QJsonObject>& jsons) const;
#else
			bool readObjects_internal(JsonArray& jsons) const;
#endif

#ifdef JD_USE_QJSON
			bool readObjects_internal(const std::vector<QJsonObject>& jsons, std::vector<JDSerializable*>& objects) const;
#else
			bool readObjects_internal(const JsonArray& jsons, std::vector<JDSerializable*>& objects) const;
#endif

			


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
#ifdef JD_USE_QJSON
			std::vector<QJsonObject> jsons;
#else
			JsonArray jsons;
#endif
			bool success = readObjects_internal(jsons);
			if(success)
			{
				objects.clear();
				objects.reserve(jsons.size());
				for(auto& json : jsons)
				{
#ifdef JD_USE_QJSON
					QJsonObject& jsonData = json;
					std::string loadedKey = LockEntryObject::getKey(jsonData);
#else
					JsonObject& jsonData = json.getObject();
					std::string loadedKey = LockEntryObject::getKey(jsonData);
#endif
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