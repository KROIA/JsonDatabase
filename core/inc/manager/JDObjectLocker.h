#pragma once

#include "JD_base.h"
#include "object/JDObjectInterface.h"
#include "object/JDSerializable.h"
#include "utilities/filesystem/FileChangeWatcher.h"

#include <string>
#include <vector>
#include <QJsonObject>
#include <QString>
#include <mutex>

namespace JsonDatabase
{
	namespace Internal
	{
		class JSONDATABASE_EXPORT JDObjectLocker
		{
			friend JDManager;
			JDObjectLocker(JDManager& manager, std::mutex &mtx);
			virtual ~JDObjectLocker();
			void setup();
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

			bool lockObject(JDObjectInterface* obj) const;
			bool unlockObject(JDObjectInterface* obj) const;
			bool unlockAllObjs() const;
			bool isObjectLocked(JDObjectInterface* obj, int &lastError) const;
			bool isObjectLockedByMe(JDObjectInterface* obj, int& lastError) const;
			bool isObjectLockedByOther(JDObjectInterface* obj, int& lastError) const;
			
			struct LockData
			{
				std::string objectID;
				std::string owner;
				std::string sessionID;
				std::string lockDate;
				std::string lockTime;
			};
			bool getLockedObjects(std::vector<LockData>& lockedObjectsOut) const;


			Error getLastObjLockError() const;
			const std::string& getLastObjLockErrorStr() const;
		protected:
			ManagedFileChangeWatcher& getLockTableFileWatcher();
			void update();

		private:
			class JSONDATABASE_EXPORT ObjectLockData : public JDSerializable
			{
			public:
				ObjectLockData();
				ObjectLockData(JDObjectInterface* obj, const JDManager &manager);
				void setObject(JDObjectInterface* obj, const JDManager& manager);
				bool load(const QJsonObject& obj) override;
				bool save(QJsonObject& obj) const override;
				static bool isValid(const QJsonObject& lock);
				std::string toString() const;

				LockData data;
				JDObjectInterface* obj;
			};

			void onDatabasePathChange(const std::string& oldPath, const std::string& newPath) const;

			
			bool readLockTable(std::vector<ObjectLockData>& locks) const;
			bool writeLockTable(const std::vector<ObjectLockData>& locks) const;

			const std::string& getTablePath() const;
			const std::string& getTableFileName() const;
			std::string getTableFileFilePath() const;

			bool getObjectLockDataFromID(const std::vector<ObjectLockData>& locks,
				const std::string& targetID,
				ObjectLockData& lockOut,
				size_t& index) const;

			void getObjectLockDataFromSessionID(const std::vector<ObjectLockData>& locks,
				const std::string& targetSessionID,
				const std::string& userName,
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


			mutable Error m_lastError;

			static QString s_jsonKey_objectID;
			static QString s_jsonKey_owner;
			static QString s_jsonKey_sessionID;
			static QString s_jsonKey_lockDate;
			static QString s_jsonKey_lockTime;
		};
	}
}