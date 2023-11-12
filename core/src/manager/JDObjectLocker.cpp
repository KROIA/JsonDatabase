#include "manager/JDManager.h"
#include "manager/JDObjectLocker.h"
#include "utilities/filesystem/FileLock.h"
#include "utilities/JDUniqueMutexLock.h"


#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDateTime>

namespace JsonDatabase
{
	namespace Internal
	{
		QString JDObjectLocker::s_jsonKey_objectID = "objID";
		QString JDObjectLocker::s_jsonKey_owner = "owner";
		QString JDObjectLocker::s_jsonKey_sessionID = "sessionID";
		QString JDObjectLocker::s_jsonKey_lockDate = "lockDate";
		QString JDObjectLocker::s_jsonKey_lockTime = "lockTime";

		JDObjectLocker::JDObjectLocker(JDManager& manager, std::mutex& mtx)
			: m_manager(manager)
			, m_mutex(mtx)
			, m_lockTableFile("lockTable.json")
			, m_lockTableTryGetLockTimeoutMs(1000)
			, m_useSpecificDatabasePath(false)
			, m_specificDatabasePath("")
			, m_lastError(Error::none)
		{

		}

		JDObjectLocker::~JDObjectLocker()
		{

		}
		void JDObjectLocker::setup()
		{

		}

		bool JDObjectLocker::lockObj(JDObjectInterface* obj) const
		{
			if (!obj)
			{
				m_lastError = Error::objIsNullptr;
				return false;
			}
			JDM_UNIQUE_LOCK_P;
			JD_OBJECT_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_10);
			std::unique_lock<std::mutex> lock(m_mutex);

			FileLock fileLock(getTableFilePath(), getTableFileName());
			if (!fileLock.lock(m_lockTableTryGetLockTimeoutMs))
			{
				m_lastError = Error::unableToLock;
				JD_CONSOLE_FUNCTION("Can't aquire lock for file: " << fileLock.getFilePath()
					<< " timeout occured after: " << m_lockTableTryGetLockTimeoutMs << "ms"
					<< " LockError: " << fileLock.getLastErrorStr() << "\n");
				return false;
			}

			std::vector<QJsonObject> locks;
			if (!readLockTable(locks))
				return false;

			// Check if Lock is already aquired 
			QJsonObject targetLock;
			size_t targetIndex;
			if (getJsonFromID(locks, obj->getObjectID(), targetLock, targetIndex))
			{
				std::string owner = targetLock[s_jsonKey_owner].toString().toStdString();
				std::string session = targetLock[s_jsonKey_sessionID].toString().toStdString();

				if (owner == m_manager.getUser() && session == m_manager.getSessionID())
				{
					// Already locked by this session
					JD_CONSOLE_FUNCTION("Lock for object: " + obj->getObjectID() + " type: " + obj->className() + " is already aquired in this session\n");
					m_lastError = Error::none;
					return true;
				}
				else
				{
					m_lastError = Error::lockedByOther;
					JD_CONSOLE_FUNCTION("Can't aquire lock for object: " + obj->getObjectID() + " type: " + obj->className() +
						"\nLock Data: \n" + toString(targetLock) + "\n"
						"Lock is already aquired from user: " + targetLock[s_jsonKey_owner].toString().toStdString() + "\n");
					return false;
				}
			}

			targetLock = getLockData(obj);
			if (!isValidJsonLock(targetLock))
			{
				m_lastError = Error::programmingError;
				JD_CONSOLE_FUNCTION("Programming error!!! ajust the JDObjectLocker::isValidJsonLock(const QJsonObject&) function\n");
				return false;
			}

			locks.push_back(targetLock);
			if (!writeLockTable(locks))
				return false;


			m_lastError = Error::none;
			return true;
		}
		bool JDObjectLocker::unlockObj(JDObjectInterface* obj) const
		{
			if (!obj)
			{
				m_lastError = Error::objIsNullptr;
				return false;
			}
			JDM_UNIQUE_LOCK_P;
			JD_OBJECT_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_10);
			std::unique_lock<std::mutex> lock(m_mutex);

			FileLock fileLock(getTableFilePath(), getTableFileName());
			if (!fileLock.lock(m_lockTableTryGetLockTimeoutMs))
			{
				m_lastError = Error::unableToLock;
				JD_CONSOLE_FUNCTION("Can't aquire lock for file: " << fileLock.getFilePath()
					<< " timeout occured after: " << m_lockTableTryGetLockTimeoutMs << "ms"
					<< " LockError: " << fileLock.getLastErrorStr() << "\n");
				return false;
			}

			std::vector<QJsonObject> locks;
			if (!readLockTable(locks))
				return false;

			// Check if Lock is already aquired 
			QJsonObject targetLock;
			size_t targetIndex;
			if (getJsonFromID(locks, obj->getObjectID(), targetLock, targetIndex))
			{
				std::string owner = targetLock[s_jsonKey_owner].toString().toStdString();
				std::string session = targetLock[s_jsonKey_sessionID].toString().toStdString();

				if (owner == m_manager.getUser() && session == m_manager.getSessionID())
				{
					// Locked by this session

					// Remove lock from list
					locks.erase(locks.begin() + targetIndex);

					// Save table
					if (!writeLockTable(locks))
						return false;
					m_lastError = Error::none;
					return true;
				}
				else
				{
					m_lastError = Error::lockedByOther;
					std::string msg = "Can't release lock for object: " + obj->getObjectID() + " type: " + obj->className();
					msg += "\nLock Data: \n" + toString(targetLock) + "\n";
					msg += "Lock is owned by user: " + targetLock[s_jsonKey_owner].toString().toStdString() + "\n";
					JD_CONSOLE_FUNCTION(msg);
					return false;
				}
			}
			JD_CONSOLE_FUNCTION("Lock for object: " + obj->getObjectID() + " type: " + obj->className() + " did not exist\n");

			m_lastError = Error::none;
			return true;
		}
		bool JDObjectLocker::unlockAllObjs() const
		{
			JDM_UNIQUE_LOCK_P;
			JD_OBJECT_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_10);
			FileLock fileLock(getTableFilePath(), getTableFileName());
			if (!fileLock.lock(m_lockTableTryGetLockTimeoutMs))
			{
				m_lastError = Error::unableToLock;
				JD_CONSOLE_FUNCTION("Can't aquire lock for file: " << fileLock.getFilePath()
					<< " timeout occured after: " << m_lockTableTryGetLockTimeoutMs << "ms"
					<< " LockError: " << fileLock.getLastErrorStr() << "\n");
				return false;
			}

			std::vector<QJsonObject> locks;
			if (!readLockTable(locks))
				return false;

			std::vector<QJsonObject> locksOut;
			std::vector<size_t> matches;
			std::vector<size_t> mismatches;
			getJsonFromSessionID(locks, m_manager.getSessionID(), locksOut, matches, mismatches);

			// Save all locks which do not match, with this session id, to a new table
			std::vector<QJsonObject> newLockTable;
			for (size_t i = 0; i < mismatches.size(); ++i)
			{
				newLockTable.push_back(locks[mismatches[i]]);
			}

			if (!writeLockTable(newLockTable))
				return false;

			m_lastError = Error::none;
			return true;
		}
		bool JDObjectLocker::isObjLocked(JDObjectInterface* obj) const
		{
			
			if (!obj)
			{
				m_lastError = Error::objIsNullptr;
				return false;
			}
			JDM_UNIQUE_LOCK_P;
			JD_OBJECT_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_5);
			std::unique_lock<std::mutex> lock(m_mutex);

			FileLock fileLock(getTableFilePath(), getTableFileName());
			if (!fileLock.lock(m_lockTableTryGetLockTimeoutMs))
			{
				m_lastError = Error::unableToLock;
				JD_CONSOLE_FUNCTION("Can't aquire lock for file: " << fileLock.getFilePath()
					<< " timeout occured after: " << m_lockTableTryGetLockTimeoutMs << "ms"
					<< " LockError: " << fileLock.getLastErrorStr());
				return false;
			}

			std::vector<QJsonObject> locks;
			if (!readLockTable(locks))
				return false;

			// Check if Lock is already aquired 
			QJsonObject targetLock;
			size_t targetIndex;
			if (getJsonFromID(locks, obj->getObjectID(), targetLock, targetIndex))
			{
				std::string owner = targetLock[s_jsonKey_owner].toString().toStdString();
				std::string session = targetLock[s_jsonKey_sessionID].toString().toStdString();

				if (owner == m_manager.getUser() && session == m_manager.getSessionID())
				{
					// Already locked by this session
					m_lastError = Error::none;
					return true;
				}
			}
			m_lastError = Error::none;
			return true;
		}


		JDObjectLocker::Error JDObjectLocker::getLastObjLockError() const
		{
			return m_lastError;
		}
		const std::string& JDObjectLocker::getLastObjLockErrorStr() const
		{
			switch (m_lastError)
			{
			case Error::none: { static const std::string msg = "JDObjectLocker::Error::none"; return msg; };
			case Error::objIsNullptr: { static const std::string msg = "JDObjectLocker::Error::objIsNullptr"; return msg; };
			case Error::cantOpenTableFile: { static const std::string msg = "JDObjectLocker::Error::cantOpenTableFile"; return msg; };
			case Error::tableFileNotExist: { static const std::string msg = "JDObjectLocker::Error::ntableFileNotExistne"; return msg; };
			case Error::corruptTableData: { static const std::string msg = "JDObjectLocker::Error::corruptTableData"; return msg; };
			case Error::unableToLock: { static const std::string msg = "JDObjectLocker::Error::unableToLock"; return msg; };
			case Error::lockedByOther: { static const std::string msg = "JDObjectLocker::Error::lockedByOther"; return msg; };
			case Error::programmingError: { static const std::string msg = "JDObjectLocker::Error::programmingError"; return msg; };
			}
			static std::string unknown;
			unknown = "Unknown JDObjectLocker Error: " + std::to_string(m_lastError);
			return unknown;
		}

		void JDObjectLocker::onDatabasePathChange(const std::string& oldPath, const std::string& newPath) const
		{
			JD_OBJECT_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_10);
			std::unique_lock<std::mutex> lock(m_mutex);

			FileLock fileLock1(oldPath, m_lockTableFile);
			if (!fileLock1.lock(m_lockTableTryGetLockTimeoutMs))
			{
				m_lastError = Error::unableToLock;
				JD_CONSOLE_FUNCTION("Can't aquire lock for file: " << fileLock1.getFilePath()
					<< " timeout occured after: " << m_lockTableTryGetLockTimeoutMs << "ms"
					<< " LockError: " << fileLock1.getLastErrorStr() << "\n");
				return;
			}
			FileLock fileLock2(newPath, m_lockTableFile);
			if (!fileLock2.lock(m_lockTableTryGetLockTimeoutMs))
			{
				m_lastError = Error::unableToLock;
				JD_CONSOLE_FUNCTION("Can't aquire lock for file: " << fileLock2.getFilePath()
					<< " timeout occured after: " << m_lockTableTryGetLockTimeoutMs << "ms"
					<< " LockError: " << fileLock2.getLastErrorStr() << "\n");
				return;
			}

			m_useSpecificDatabasePath = true;
			m_specificDatabasePath = oldPath;

			std::vector<QJsonObject> locks;
			if (!readLockTable(locks))
			{
				m_useSpecificDatabasePath = false;
				return;
			}

			std::vector<QJsonObject> locksOut;
			std::vector<size_t> matches;
			std::vector<size_t> mismatches;
			getJsonFromSessionID(locks, m_manager.getSessionID(), locksOut, matches, mismatches);

			std::vector<QJsonObject> locksFromOldLocation;
			for (size_t i = 0; i < mismatches.size(); ++i)
			{
				locksFromOldLocation.push_back(locks[mismatches[i]]);
			}
			writeLockTable(locksFromOldLocation);


			m_specificDatabasePath = newPath;
			std::vector<QJsonObject> locksFromNewLocation;

			if (!readLockTable(locksFromNewLocation))
			{
				m_useSpecificDatabasePath = false;
				return;
			}

			for (size_t i = 0; i < matches.size(); ++i)
			{
				locksFromNewLocation.push_back(locks[matches[i]]);
			}

			if (!writeLockTable(locksFromNewLocation))
			{
				m_useSpecificDatabasePath = false;
				return;
			}

			m_useSpecificDatabasePath = false;
			m_lastError = Error::none;
			return;
		}

		QJsonObject JDObjectLocker::getLockData(JDObjectInterface* obj) const
		{
			JD_OBJECT_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_11);
			QJsonObject data;
			data[s_jsonKey_objectID] = obj->getObjectID().c_str();
			data[s_jsonKey_owner] = m_manager.getUser().c_str();
			data[s_jsonKey_sessionID] = m_manager.getSessionID().c_str();
			data[s_jsonKey_lockDate] = QDate::currentDate().toString(Qt::DateFormat::ISODate);
			data[s_jsonKey_lockTime] = QTime::currentTime().toString(Qt::DateFormat::ISODate);

			return data;
		}
		bool JDObjectLocker::isValidJsonLock(const QJsonObject& lock) const
		{
			JD_OBJECT_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_11);
			if (!lock.contains(s_jsonKey_objectID)) return false;
			if (!lock.contains(s_jsonKey_owner)) return false;
			if (!lock.contains(s_jsonKey_sessionID)) return false;
			if (!lock.contains(s_jsonKey_lockDate)) return false;
			if (!lock.contains(s_jsonKey_lockTime)) return false;

			return true;
		}
		std::string JDObjectLocker::toString(QJsonObject& obj) const
		{
			QJsonDocument jsonDoc(obj);

			// Convert the JSON document to a string
			QString jsonString = jsonDoc.toJson(QJsonDocument::Indented);
			return jsonString.toStdString();
		}

		bool JDObjectLocker::readLockTable(std::vector<QJsonObject>& locks) const
		{
			JD_OBJECT_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_11);
			QFile file(getTableFilePath().c_str());
			if (!file.exists())
			{
				JD_CONSOLE_FUNCTION("File: " << getTableFilePath().c_str() << " does not exist\n");
				//m_lastError = Error::tableFileNotExist;
				if (file.open(QIODevice::WriteOnly))
					file.close();
				return true;
			}
			if (!file.open(QIODevice::ReadOnly)) {
				JD_CONSOLE_FUNCTION("Failed to open file for reading: " << getTableFilePath().c_str() << "\n");
				m_lastError = Error::cantOpenTableFile;
				return false;
			}

			std::string jsonData = file.readAll();

			QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData.c_str());
			if (!jsonDoc.isNull()) {
				if (!jsonDoc.isArray())
				{
					JD_CONSOLE_FUNCTION("Tabledata: " << jsonData.c_str() << " is not a QJsonArray\n");
					m_lastError = Error::corruptTableData;
					return false;
				}
				QJsonArray array = jsonDoc.array();
				for (const QJsonValue& value : array)
				{
					if (!value.isObject())
					{
						JD_CONSOLE_FUNCTION("Tabledata value: " << value.toString().toStdString().c_str() << " is not a QJsonObject\n");
						m_lastError = Error::corruptTableData;
						return false;
					}
					QJsonObject lock = value.toObject();
					if (!isValidJsonLock(lock))
					{
						JD_CONSOLE_FUNCTION("Tabledata value: " << value.toString().toStdString().c_str() << " is has missing or corrupt data\n");
						m_lastError = Error::corruptTableData;
						return false;
					}
					locks.push_back(lock);
				}
			}
			return true;
		}
		bool JDObjectLocker::writeLockTable(const std::vector<QJsonObject>& locks) const
		{
			JD_OBJECT_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_11);
			QFile file(getTableFilePath().c_str());
			if (!file.open(QIODevice::WriteOnly)) {
				JD_CONSOLE_FUNCTION("Failed to open file for writing: " << getTableFilePath().c_str() << "\n");
				m_lastError = Error::cantOpenTableFile;
				return false;
			}
			QJsonArray array;
			for (const QJsonObject& lock : locks)
				array.append(lock);

			QJsonDocument jsonDoc(array);
			QByteArray transmittStr = jsonDoc.toJson(QJsonDocument::Compact);

			file.write(transmittStr);
			file.close();
			return true;
		}
		const std::string& JDObjectLocker::getTableFilePath() const
		{
			if (m_useSpecificDatabasePath)
			{
				return m_specificDatabasePath;
			}
			return m_manager.getDatabasePath();
		}
		const std::string& JDObjectLocker::getTableFileName() const
		{
			return m_lockTableFile;
		}

		bool JDObjectLocker::getJsonFromID(const std::vector<QJsonObject>& locks,
			const std::string& targetID,
			QJsonObject& lockOut,
			size_t& index) const
		{
			JD_OBJECT_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_11);
			for (size_t i = 0; i < locks.size(); ++i)
			{
				const QJsonObject& lock = locks[i];
				if (lock.contains(s_jsonKey_objectID))
				{
					std::string id = lock[s_jsonKey_objectID].toString().toStdString();
					if (id == targetID)
					{
						lockOut = lock;
						index = i;
						return true;
					}
				}
			}
			return false;
		}
		void JDObjectLocker::getJsonFromSessionID(const std::vector<QJsonObject>& locks,
			const std::string& targetSessionID,
			std::vector<QJsonObject>& locksOut,
			std::vector<size_t>& matches,
			std::vector<size_t>& mismatches) const
		{
			JD_OBJECT_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_11);
			for (size_t i = 0; i < locks.size(); ++i)
			{
				const QJsonObject& lock = locks[i];
				if (lock.contains(s_jsonKey_sessionID))
				{
					std::string id = lock[s_jsonKey_sessionID].toString().toStdString();
					if (id == targetSessionID)
					{
						locksOut.push_back(lock);
						matches.push_back(i);
					}
					else
					{
						mismatches.push_back(i);
					}
				}
			}
		}
	}

	
}