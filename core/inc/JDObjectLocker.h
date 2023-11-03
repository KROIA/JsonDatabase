#pragma once

#include "JD_base.h"
#include "JDObjectInterface.h"

#include <string>
#include <vector>
#include <QJsonObject>
#include <QString>
#include <mutex>

namespace JsonDatabase
{
	class JDManager;
	class JSONDATABASE_EXPORT JDObjectLocker
	{
		friend JDManager;
		JDObjectLocker(JDManager* manager);
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

		bool lock(JDObjectInterface* obj) const;
		bool unlock(JDObjectInterface* obj) const;
		bool unlockAll() const;
		bool isLocked(JDObjectInterface* obj) const;


		Error getLastError() const;
		const std::string& getLastErrorStr() const;
	private:

		void onDatabasePathChange(const std::string& oldPath, const std::string& newPath) const;

		QJsonObject getLockData(JDObjectInterface* obj) const;
		bool isValidJsonLock(const QJsonObject& lock) const;
		std::string toString(QJsonObject& obj) const;

		bool readLockTable(std::vector<QJsonObject>& locks) const;
		bool writeLockTable(const std::vector<QJsonObject>& locks) const;

		std::string getTableFilePath() const;

		bool getJsonFromID(const std::vector<QJsonObject>& locks, 
						   const std::string& targetID,
						   QJsonObject& lockOut, 
						   size_t &index) const;

		void getJsonFromSessionID(const std::vector<QJsonObject>& locks,
			const std::string& targetSessionID,
			std::vector<QJsonObject>& locksOut,
			std::vector<size_t>& matches,
			std::vector<size_t>& mismatches) const;
		

		JDManager* m_manager;
		std::string m_lockTableFile;
		unsigned int m_lockTableTryGetLockTimeoutMs;
		mutable bool m_useSpecificDatabasePath;
		mutable std::string m_specificDatabasePath;
		mutable std::mutex m_mutex;


		mutable Error m_lastError;

		static QString s_jsonKey_objectID;
		static QString s_jsonKey_owner;
		static QString s_jsonKey_sessionID;
		static QString s_jsonKey_lockDate;
		static QString s_jsonKey_lockTime;
	};
}