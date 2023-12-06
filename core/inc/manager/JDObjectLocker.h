#pragma once

#include "JD_base.h"
#include "object/JDObjectInterface.h"
#include "utilities/JDSerializable.h"
#include "utilities/filesystem/FileChangeWatcher.h"
#include "utilities/JDUser.h"
#include "utilities/filesystem/AbstractRegistry.h"


#include <string>
#include <vector>
#ifdef JD_USE_QJSON
#include <QJsonObject>
#include <QString>
#else
#include "Json/JsonValue.h"
#endif
#include <mutex>

namespace JsonDatabase
{
	namespace Internal
	{
		class JSONDATABASE_EXPORT JDObjectLocker : public Utilities::AbstractRegistry
		{
			//friend JDManagerObjectManager;
		public:
			enum Error
			{
				none,
				objIsNullptr,
				cantOpenTableFile,
				tableFileNotExist,
				corruptTableData,
				unableToLock,
				lockedByOther,
				programmingError,
			};

		protected:
		
			
			//bool setup(Error &err);
		public:
			JDObjectLocker(JDManager& manager, std::mutex& mtx);
			~JDObjectLocker();

			bool lockObject(const JDObject & obj, Error& err);
			bool unlockObject(const JDObject & obj, Error& err);
			bool unlockAllObjs(Error& err);
			bool isObjectLocked(const JDObject & obj, Error& err) const;
			bool isObjectLockedByMe(const JDObject & obj, Error& err) const;
			bool isObjectLockedByOther(const JDObject & obj, Error& err) const;

			
			struct LockData
			{
				JDObjectID::IDType objectID;
				Utilities::JDUser user;
				QDate lockDate;
				QTime lockTime;
			};
			bool getLockedObjects(std::vector<LockData>& lockedObjectsOut, Error& err) const;
			int removeInactiveObjectLocks() const;


			const std::string& getErrorStr(Error err) const;

			struct JsonKeys
			{
				static const std::string objectID;
				static const std::string user;
				static const std::string lockDate;
				static const std::string lockTime;
			};

			void update();
		protected:
			ManagedFileChangeWatcher& getLockTableFileWatcher();
			
			//void onDatabasePathChange(const std::string& oldPath, const std::string& newPath, Error& err) const;


		private:
			class JSONDATABASE_EXPORT LockEntryObjectImpl : public LockEntryObject
			{
			public:
				LockEntryObjectImpl(const std::string& key);
				LockEntryObjectImpl(const std::string& key, const JDObject& obj, const JDManager& manager);
				~LockEntryObjectImpl();


				void setObject(const JDObject& obj, const JDManager& manager);
				std::string toString() const;
				

#ifdef JD_USE_QJSON
				bool load(const QJsonObject& obj) override;
				bool save(QJsonObject& obj) const override;
				static bool isValid(const QJsonObject& lock);
#else
				bool load(const JsonObject& obj) override;
				bool save(JsonObject& obj) const override;
				static bool isValid(const JsonObject& lock);
#endif

			
				LockData data;
				JDObject obj;
			private:
			};


			void onCreateFiles() override;
			void onDatabasePathChangeStart(const std::string& newPath) override;
			void onDatabasePathChangeEnd() override;
			void onNameChange(const std::string& newName) override;





			/*
			class JSONDATABASE_EXPORT ObjectLockData : public Utilities::JDSerializable
			{
			public:
				ObjectLockData();
				ObjectLockData(const JDObject & obj, const JDManager &manager);
				void setObject(const JDObject & obj, const JDManager& manager);
#ifdef JD_USE_QJSON
				bool load(const QJsonObject& obj) override;
				bool save(QJsonObject& obj) const override;

				static bool isValid(const QJsonObject& lock);
#else
				bool load(const JsonObject& obj) override;
				bool save(JsonObject &obj) const override;

				static bool isValid(const JsonObject& lock);
#endif

				std::string toString() const;

				LockData data;
				JDObject obj;
			};*/

			
			
			//bool readLockTable(std::vector<ObjectLockData>& locks, Error& err) const;
			//bool writeLockTable(const std::vector<ObjectLockData>& locks, Error& err) const;

			//std::string getTablePath() const;
			//const std::string& getTableFileName() const;
			//std::string getTableFileFilePath() const;

			/*bool getObjectLockDataFromID(const std::vector<ObjectLockData>& locks,
				const JDObjectIDptr& targetID,
				ObjectLockData& lockOut,
				size_t& index) const;

			void getObjectLockDataFromSessionID(const std::vector<ObjectLockData>& locks,
				const std::string& targetSessionID,
				//const Utilities::JDUser& user,
				std::vector<ObjectLockData>& locksOut,
				std::vector<size_t>& matches,
				std::vector<size_t>& mismatches) const;
				*/

			JDManager& m_manager;
			std::mutex& m_mutex;
			std::string m_lockTableFile;
			unsigned int m_registryOpenTimeoutMs;
			mutable bool m_useSpecificDatabasePath;
			mutable std::string m_specificDatabasePath;
			
			mutable ManagedFileChangeWatcher m_lockTableWatcher;
		};
	}
}