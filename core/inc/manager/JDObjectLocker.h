#pragma once

#include "JD_base.h"
#include "object/JDObjectInterface.h"
#include "utilities/JDSerializable.h"
#include "utilities/filesystem/FileChangeWatcher.h"
#include "utilities/JDUser.h"

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
		class JSONDATABASE_EXPORT JDObjectLocker
		{
			//friend JDManager;
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
			
			JDObjectLocker(JDManager& manager, std::mutex &mtx);
			virtual ~JDObjectLocker();
			bool setup(Error &err);
		public:
			

			bool lockObject(const JDObject & obj, Error& err) const;
			bool unlockObject(const JDObject & obj, Error& err) const;
			bool unlockAllObjs(Error& err) const;
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


			const std::string& getErrorStr(Error err) const;

			struct JsonKeys
			{
				static constexpr std::string_view objectID = "objID";
				static constexpr std::string_view user     = "user";
				static constexpr std::string_view lockDate = "lockDate";
				static constexpr std::string_view lockTime = "lockTime";
			};

		protected:
			ManagedFileChangeWatcher& getLockTableFileWatcher();
			void update();
			void onDatabasePathChange(const std::string& oldPath, const std::string& newPath, Error& err) const;


		private:
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
			};

			
			
			bool readLockTable(std::vector<ObjectLockData>& locks, Error& err) const;
			bool writeLockTable(const std::vector<ObjectLockData>& locks, Error& err) const;

			const std::string& getTablePath() const;
			const std::string& getTableFileName() const;
			std::string getTableFileFilePath() const;

			bool getObjectLockDataFromID(const std::vector<ObjectLockData>& locks,
				const JDObjectIDptr& targetID,
				ObjectLockData& lockOut,
				size_t& index) const;

			void getObjectLockDataFromSessionID(const std::vector<ObjectLockData>& locks,
				const std::string& targetSessionID,
				//const Utilities::JDUser& user,
				std::vector<ObjectLockData>& locksOut,
				std::vector<size_t>& matches,
				std::vector<size_t>& mismatches) const;


			JDManager& m_manager;
			std::mutex& m_mutex;
			std::string m_lockTableFile;
			unsigned int m_lockTableTryGetLockTimeoutMs;
			mutable bool m_useSpecificDatabasePath;
			mutable std::string m_specificDatabasePath;
			
			mutable ManagedFileChangeWatcher m_lockTableWatcher;
		};
	}
}