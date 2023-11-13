#pragma once

#include "JD_base.h"
#include "object/JDObjectInterface.h"

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
			bool isObjectLocked(JDObjectInterface* obj) const;


			Error getLastObjLockError() const;
			const std::string& getLastObjLockErrorStr() const;
		private:

			void onDatabasePathChange(const std::string& oldPath, const std::string& newPath) const;

			QJsonObject getLockData(JDObjectInterface* obj) const;
			bool isValidJsonLock(const QJsonObject& lock) const;
			std::string toString(QJsonObject& obj) const;

			bool readLockTable(std::vector<QJsonObject>& locks) const;
			bool writeLockTable(const std::vector<QJsonObject>& locks) const;

			const std::string& getTablePath() const;
			const std::string& getTableFileName() const;
			std::string getTableFileFilePath() const;

			bool getJsonFromID(const std::vector<QJsonObject>& locks,
				const std::string& targetID,
				QJsonObject& lockOut,
				size_t& index) const;

			void getJsonFromSessionID(const std::vector<QJsonObject>& locks,
				const std::string& targetSessionID,
				const std::string& userName,
				std::vector<QJsonObject>& locksOut,
				std::vector<size_t>& matches,
				std::vector<size_t>& mismatches) const;


			JDManager& m_manager;
			std::mutex& m_mutex;
			std::string m_lockTableFile;
			unsigned int m_lockTableTryGetLockTimeoutMs;
			mutable bool m_useSpecificDatabasePath;
			mutable std::string m_specificDatabasePath;
			


			mutable Error m_lastError;

			static QString s_jsonKey_objectID;
			static QString s_jsonKey_owner;
			static QString s_jsonKey_sessionID;
			static QString s_jsonKey_lockDate;
			static QString s_jsonKey_lockTime;
		};
	}
}