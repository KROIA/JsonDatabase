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
			unlockAllObjs();
		}
		void JDObjectLocker::setup()
		{
			JDM_UNIQUE_LOCK_P;
			JD_OBJECT_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_10);

			m_lastError = Error::none;
			FileLock fileLock(getTablePath(), getTableFileName());
			if (!fileLock.lock(m_lockTableTryGetLockTimeoutMs))
			{
				m_lastError = Error::unableToLock;
				JD_CONSOLE_FUNCTION("Can't aquire lock for file: " << fileLock.getFilePath()
					<< " timeout occured after: " << m_lockTableTryGetLockTimeoutMs << "ms"
					<< " LockError: " << fileLock.getLastErrorStr() << "\n");
				return;
			}
			QFile file(getTableFileFilePath().c_str());
			if (!file.exists())
			{
				writeLockTable({});					
			}

			m_lockTableWatcher.setup(getTableFileFilePath());
		}

		bool JDObjectLocker::lockObject(JDObjectInterface* obj) const
		{
			if (!obj)
			{
				m_lastError = Error::objIsNullptr;
				return false;
			}
			JDM_UNIQUE_LOCK_P;
			JD_OBJECT_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_10);

			FileLock fileLock(getTablePath(), getTableFileName());
			if (!fileLock.lock(m_lockTableTryGetLockTimeoutMs))
			{
				m_lastError = Error::unableToLock;
				JD_CONSOLE("bool JDObjectLocker::lockObject(obj:\""<<obj->getObjectID()<<"\") Can't aquire lock for file: " << fileLock.getFilePath()
					<< " timeout occured after: " << m_lockTableTryGetLockTimeoutMs << "ms"
					<< " LockError: " << fileLock.getLastErrorStr() << "\n");
				return false;
			}

			std::vector<ObjectLockData> locks;
			if (!readLockTable(locks))
				return false;

			// Check if Lock is already aquired 
			ObjectLockData targetLock;
			size_t targetIndex;
			if (getObjectLockDataFromID(locks, obj->getObjectID(), targetLock, targetIndex))
			{
				if (targetLock.data.owner == m_manager.getUser() && 
					targetLock.data.sessionID == m_manager.getSessionID())
				{
					// Already locked by this session
					JD_CONSOLE("bool JDObjectLocker::lockObject(obj:\"" << obj->getObjectID() << "\") Lock for object: \"" + obj->getObjectID() + "\" type: \"" + obj->className() + "\" is already aquired in this session\n");
					m_lastError = Error::none;
					return true;
				}
				else
				{
					m_lastError = Error::lockedByOther;
					JD_CONSOLE("bool JDObjectLocker::lockObject(obj:\"" << obj->getObjectID() << "\") Can't aquire lock for object: \"" + obj->getObjectID() + "\" type: \"" + obj->className() +
						"\"\nLock Data: \n" + targetLock.toString() + "\n"
						"Lock is already aquired from user: \"" + targetLock.data.owner + "\"\n");
					return false;
				}
			}

			targetLock.setObject(obj, m_manager);
			

			locks.push_back(targetLock);
			if (!writeLockTable(locks))
				return false;


			// Verify lock success
			{
				JD_OBJECT_LOCK_PROFILING_BLOCK("Verify object lock", JD_COLOR_STAGE_11);
				std::vector<ObjectLockData> verifyLocks;
				if (!readLockTable(verifyLocks))
					return false;

				// Check if Lock is already aquired 
				ObjectLockData targetLock;
				size_t targetIndex;
				if (getObjectLockDataFromID(locks, obj->getObjectID(), targetLock, targetIndex))
				{
					if (targetLock.data.owner == m_manager.getUser() &&
						targetLock.data.sessionID == m_manager.getSessionID())
					{
						// Already locked by this session
						m_lastError = Error::none;
						return true;
					}
					else
					{
						m_lastError = Error::lockedByOther;
						JD_CONSOLE("Can't verify the locked object: " << obj->getObjectID() <<" Object is locked by: \""<< targetLock.data.owner <<"\"");
						return false;
					}
				}
				else
				{
					m_lastError = Error::unableToLock;
					JD_CONSOLE("Can't verify the locked object: "<< obj->getObjectID());
					return false;
				}
			}

			m_lastError = Error::none;
			return true;
		}
		bool JDObjectLocker::unlockObject(JDObjectInterface* obj) const
		{
			if (!obj)
			{
				m_lastError = Error::objIsNullptr;
				return false;
			}
			JDM_UNIQUE_LOCK_P;
			JD_OBJECT_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_10);


			FileLock fileLock(getTablePath(), getTableFileName());
			if (!fileLock.lock(m_lockTableTryGetLockTimeoutMs))
			{
				m_lastError = Error::unableToLock;
				JD_CONSOLE_FUNCTION("Can't aquire lock for file: " << fileLock.getFilePath()
					<< " timeout occured after: " << m_lockTableTryGetLockTimeoutMs << "ms"
					<< " LockError: " << fileLock.getLastErrorStr() << "\n");
				return false;
			}

			std::vector<ObjectLockData> locks;
			if (!readLockTable(locks))
				return false;

			// Check if Lock is already aquired 
			ObjectLockData targetLock;
			size_t targetIndex;
			if (getObjectLockDataFromID(locks, obj->getObjectID(), targetLock, targetIndex))
			{
				if (targetLock.data.owner == m_manager.getUser() &&
					targetLock.data.sessionID == m_manager.getSessionID())
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
					
					JD_CONSOLE_FUNCTION("Can't release lock for object: \"" + obj->getObjectID() + "\" type: \"" + obj->className()
					<<"\"\nLock Data: \n" + targetLock.toString() + "\n"
					<<"Lock is owned by user: " + targetLock.data.owner + "\n");
					return false;
				}
			}
			JD_CONSOLE_FUNCTION("Lock for object: \"" + obj->getObjectID() + "\" type: \"" + obj->className() + "\" did not exist\n");

			m_lastError = Error::none;
			return true;
		}
		bool JDObjectLocker::unlockAllObjs() const
		{
			JDM_UNIQUE_LOCK_P;
			JD_OBJECT_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_10);
			FileLock fileLock(getTablePath(), getTableFileName());
			if (!fileLock.lock(m_lockTableTryGetLockTimeoutMs))
			{
				m_lastError = Error::unableToLock;
				JD_CONSOLE_FUNCTION("Can't aquire lock for file: " << fileLock.getFilePath()
					<< " timeout occured after: " << m_lockTableTryGetLockTimeoutMs << "ms"
					<< " LockError: " << fileLock.getLastErrorStr() << "\n");
				return false;
			}

			std::vector<ObjectLockData> locks;
			if (!readLockTable(locks))
				return false;

			std::vector<ObjectLockData> locksOut;
			std::vector<size_t> matches;
			std::vector<size_t> mismatches;
			getObjectLockDataFromSessionID(locks, m_manager.getSessionID(), m_manager.getUser(), locksOut, matches, mismatches);

			// Save all locks which do not match, with this session id, to a new table
			std::vector<ObjectLockData> newLockTable;
			for (size_t i = 0; i < mismatches.size(); ++i)
			{
				newLockTable.push_back(locks[mismatches[i]]);
			}

			if (!writeLockTable(newLockTable))
				return false;

			m_lastError = Error::none;
			return true;
		}
		bool JDObjectLocker::isObjectLocked(JDObjectInterface* obj, int& lastError) const
		{
			if (!obj)
			{
				m_lastError = Error::objIsNullptr;
				lastError = m_lastError;
				return false;
			}
			JDM_UNIQUE_LOCK_P;
			JD_OBJECT_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_5);

			FileLock fileLock(getTablePath(), getTableFileName());
			if (!fileLock.lock(m_lockTableTryGetLockTimeoutMs))
			{
				m_lastError = Error::unableToLock;
				JD_CONSOLE_FUNCTION("Can't aquire lock for file: " << fileLock.getFilePath() << "\n"
					<< " timeout occured after: " << m_lockTableTryGetLockTimeoutMs << "ms\n"
					<< " LockError: " << fileLock.getLastErrorStr() << "\n");
				lastError = m_lastError;
				return false;
			}

			std::vector<ObjectLockData> locks;
			if (!readLockTable(locks))
			{
				lastError = m_lastError;
				return false;
			}
			// Check if Lock is already aquired 
			ObjectLockData targetLock;
			size_t targetIndex;
			if (getObjectLockDataFromID(locks, obj->getObjectID(), targetLock, targetIndex))
			{
				m_lastError = Error::none;
				lastError = m_lastError;
				return true; // Object is locked by any user
			}
			m_lastError = Error::none;
			lastError = m_lastError;
			return false;
		}
		bool JDObjectLocker::isObjectLockedByMe(JDObjectInterface* obj, int& lastError) const
		{
			if (!obj)
			{
				m_lastError = Error::objIsNullptr;
				lastError = m_lastError;
				return false;
			}
			JDM_UNIQUE_LOCK_P;
			JD_OBJECT_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_5);

			FileLock fileLock(getTablePath(), getTableFileName());
			if (!fileLock.lock(m_lockTableTryGetLockTimeoutMs))
			{
				m_lastError = Error::unableToLock;
				JD_CONSOLE_FUNCTION("Can't aquire lock for file: " << fileLock.getFilePath() << "\n"
					<< " timeout occured after: " << m_lockTableTryGetLockTimeoutMs << "ms\n"
					<< " LockError: " << fileLock.getLastErrorStr() << "\n");
				lastError = m_lastError;
				return false;
			}

			std::vector<ObjectLockData> locks;
			if (!readLockTable(locks))
			{
				lastError = m_lastError;
				return false;
			}

			// Check if Lock is already aquired 
			ObjectLockData targetLock;
			size_t targetIndex;
			if (getObjectLockDataFromID(locks, obj->getObjectID(), targetLock, targetIndex))
			{
				if (targetLock.data.owner == m_manager.getUser() &&
					targetLock.data.sessionID == m_manager.getSessionID())
				{
					// Already locked by this session
					m_lastError = Error::none;
					lastError = m_lastError;
					return true;
				}
			}
			m_lastError = Error::none;
			lastError = m_lastError;
			return false;
		}
		bool JDObjectLocker::isObjectLockedByOther(JDObjectInterface* obj, int& lastError) const
		{
			if (!obj)
			{
				m_lastError = Error::objIsNullptr;
				lastError = m_lastError;
				return false;
			}
			JDM_UNIQUE_LOCK_P;
			JD_OBJECT_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_5);

			FileLock fileLock(getTablePath(), getTableFileName());
			if (!fileLock.lock(m_lockTableTryGetLockTimeoutMs))
			{
				m_lastError = Error::unableToLock;
				JD_CONSOLE_FUNCTION("Can't aquire lock for file: " << fileLock.getFilePath() << "\n"
					<< " timeout occured after: " << m_lockTableTryGetLockTimeoutMs << "ms\n"
					<< " LockError: " << fileLock.getLastErrorStr() << "\n");
				lastError = m_lastError;
				return false;
			}

			std::vector<ObjectLockData> locks;
			if (!readLockTable(locks))
			{
				lastError = m_lastError;
				return false;
			}

			// Check if Lock is already aquired 
			ObjectLockData targetLock;
			size_t targetIndex;
			if (getObjectLockDataFromID(locks, obj->getObjectID(), targetLock, targetIndex))
			{
				if (targetLock.data.owner != m_manager.getUser() ||
					targetLock.data.sessionID != m_manager.getSessionID())
				{
					// Already locked by this session
					m_lastError = Error::none;
					lastError = m_lastError;
					return true;
				}
			}
			m_lastError = Error::none;
			lastError = m_lastError;
			return false;
		}
		bool JDObjectLocker::getLockedObjects(std::vector<LockData>& lockedObjectsOut) const
		{
			JDM_UNIQUE_LOCK_P;
			JD_OBJECT_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_5);
			lockedObjectsOut.clear();

			FileLock fileLock(getTablePath(), getTableFileName());
			if (!fileLock.lock(m_lockTableTryGetLockTimeoutMs))
			{
				m_lastError = Error::unableToLock;
				JD_CONSOLE_FUNCTION("Can't aquire lock for file: " << fileLock.getFilePath() << "\n"
					<< " timeout occured after: " << m_lockTableTryGetLockTimeoutMs << "ms\n"
					<< " LockError: " << fileLock.getLastErrorStr() << "\n");
				return false;
			}

			std::vector<ObjectLockData> locks;
			if (!readLockTable(locks))
			{
				return false;
			}

			lockedObjectsOut.reserve(locks.size());
			bool success = true;
			for (const ObjectLockData& lock : locks)
			{
				if (lock.data.objectID.size() > 0)
					lockedObjectsOut.push_back(lock.data);
				else
				{
					JD_CONSOLE_FUNCTION("Object has empty objectID: "
						<< lock.toString() << "\n");
					m_lastError = corruptTableData;
					success = false;
				}
			}
			if(success)
				m_lastError = Error::none;
			return success;
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

		ManagedFileChangeWatcher& JDObjectLocker::getLockTableFileWatcher()
		{
			return m_lockTableWatcher;
		}

		void JDObjectLocker::update()
		{
			// m_lockTableWatcher
			if (m_lockTableWatcher.hasFileChanged())
			{
				m_lockTableWatcher.clearHasFileChanged();
				m_manager.onObjectLockerFileChanged();
				m_manager.getSignals().lockedObjectsChanged.emitSignal();
			}
		}


		JDObjectLocker::ObjectLockData::ObjectLockData()
			: obj(nullptr)
		{
			
		}
		JDObjectLocker::ObjectLockData::ObjectLockData(JDObjectInterface* obj, const JDManager& manager)
			: obj(nullptr)
		{
			setObject(obj, manager);
		}
		void JDObjectLocker::ObjectLockData::setObject(JDObjectInterface* obj, const JDManager& manager)
		{
			this->obj = obj;
			if (!this->obj)
				return;
			data.objectID = this->obj->getObjectID();
			data.owner = manager.getUser();
			data.sessionID = manager.getSessionID();
			data.lockDate = QDate::currentDate().toString(Qt::DateFormat::ISODate).toStdString();
			data.lockTime = QTime::currentTime().toString(Qt::DateFormat::ISODate).toStdString();
		}
		bool JDObjectLocker::ObjectLockData::load(const QJsonObject& obj)
		{
			if(!isValid(obj))
				return false;
			data.objectID = obj[s_jsonKey_objectID].toString().toStdString();
			data.owner = obj[s_jsonKey_owner].toString().toStdString();
			data.sessionID = obj[s_jsonKey_sessionID].toString().toStdString();
			data.lockDate = obj[s_jsonKey_lockDate].toString().toStdString();
			data.lockTime = obj[s_jsonKey_lockTime].toString().toStdString();
			return true;
		}
		bool JDObjectLocker::ObjectLockData::save(QJsonObject& obj) const
		{
			JD_OBJECT_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_11);

			obj[s_jsonKey_objectID] = data.objectID.c_str();
			obj[s_jsonKey_owner] = data.owner.c_str();
			obj[s_jsonKey_sessionID] = data.sessionID.c_str();
			obj[s_jsonKey_lockDate] = data.lockDate.c_str();
			obj[s_jsonKey_lockTime] = data.lockTime.c_str();
			return true;
		}
		bool JDObjectLocker::ObjectLockData::isValid(const QJsonObject& lock)
		{
			JD_OBJECT_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_11);
			if (!lock.contains(s_jsonKey_objectID)) return false;
			if (!lock.contains(s_jsonKey_owner)) return false;
			if (!lock.contains(s_jsonKey_sessionID)) return false;
			if (!lock.contains(s_jsonKey_lockDate)) return false;
			if (!lock.contains(s_jsonKey_lockTime)) return false;

			return true;
		}
		std::string JDObjectLocker::ObjectLockData::toString() const
		{
			QJsonObject obj;
			save(obj);
			QJsonDocument jsonDoc(obj);

			// Convert the JSON document to a string
			QString jsonString = jsonDoc.toJson(QJsonDocument::Indented);
			return jsonString.toStdString();
		}


		void JDObjectLocker::onDatabasePathChange(const std::string& oldPath, const std::string& newPath) const
		{
			JD_OBJECT_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_10);

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
			m_lockTableWatcher.stop();

			m_useSpecificDatabasePath = true;
			m_specificDatabasePath = oldPath;

			std::vector<ObjectLockData> locks;
			if (!readLockTable(locks))
			{
				m_useSpecificDatabasePath = false;
				return;
			}

			std::vector<ObjectLockData> locksOut;
			std::vector<size_t> matches;
			std::vector<size_t> mismatches;
			getObjectLockDataFromSessionID(locks, m_manager.getSessionID(), m_manager.getUser(), locksOut, matches, mismatches);

			std::vector<ObjectLockData> locksFromOldLocation;
			for (size_t i = 0; i < mismatches.size(); ++i)
			{
				locksFromOldLocation.push_back(locks[mismatches[i]]);
			}
			writeLockTable(locksFromOldLocation);


			m_specificDatabasePath = newPath;
			std::vector<ObjectLockData> locksFromNewLocation;

			if (!readLockTable(locksFromNewLocation))
			{
				m_useSpecificDatabasePath = false;
				m_lockTableWatcher.setup(getTableFileFilePath());
				return;
			}

			for (size_t i = 0; i < matches.size(); ++i)
			{
				locksFromNewLocation.push_back(locks[matches[i]]);
			}

			if (!writeLockTable(locksFromNewLocation))
			{
				m_useSpecificDatabasePath = false;
				m_lockTableWatcher.setup(getTableFileFilePath());
				return;
			}

			m_useSpecificDatabasePath = false;
			m_lastError = Error::none;
			m_lockTableWatcher.setup(getTableFileFilePath());
			return;
		}
		
		

		bool JDObjectLocker::readLockTable(std::vector<ObjectLockData>& locks) const
		{
			JD_OBJECT_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_11);
			std::string filePath = getTableFileFilePath();
			QFile file(filePath.c_str());
			if (!file.exists())
			{
				JD_CONSOLE_FUNCTION("File: " << filePath.c_str() << " does not exist\n");
				//m_lastError = Error::tableFileNotExist;
				if (file.open(QIODevice::WriteOnly))
					file.close();
				return true;
			}
			if (!file.open(QIODevice::ReadOnly)) {
				JD_CONSOLE_FUNCTION("Failed to open file for reading: " << filePath.c_str() << "\n");
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
					if (!ObjectLockData::isValid(lock))
					{
						JD_CONSOLE_FUNCTION("Tabledata value: " << value.toString().toStdString().c_str() << " is has missing or corrupt data\n");
						m_lastError = Error::corruptTableData;
						return false;
					}

					locks.push_back(ObjectLockData());
					locks[locks.size()-1].load(lock);
				}
			}
			return true;
		}
		bool JDObjectLocker::writeLockTable(const std::vector<ObjectLockData>& locks) const
		{
			JD_OBJECT_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_11);
			std::string filePath = getTableFileFilePath();
			
			QFile file(filePath.c_str());
			if (!file.open(QIODevice::WriteOnly)) {
				JD_CONSOLE_FUNCTION("Failed to open file for writing: " << filePath.c_str() << "\n");
				m_lastError = Error::cantOpenTableFile;
				return false;
			}
			QJsonArray array;
			for (const ObjectLockData& lock : locks)
			{
				QJsonObject sav;
				lock.save(sav);
				if (!ObjectLockData::isValid(sav))
				{
					m_lastError = Error::programmingError;
					JD_CONSOLE_FUNCTION("Programming error!!! ajust the ObjectLockData::isValid(const QJsonObject&) function\n");
					return false;
				}
				array.append(sav);
			}

			QJsonDocument jsonDoc(array);
			QByteArray transmittStr = jsonDoc.toJson(QJsonDocument::Indented);


			//m_lockTableWatcher.pause();

			file.write(transmittStr);
			file.close();

			//m_lockTableWatcher.unpause();
			return true;
		}
		const std::string& JDObjectLocker::getTablePath() const
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
		std::string JDObjectLocker::getTableFileFilePath() const
		{
			return getTablePath() + "\\" + getTableFileName();
		}

		bool JDObjectLocker::getObjectLockDataFromID(const std::vector<ObjectLockData>& locks,
			const std::string& targetID,
			ObjectLockData& lockOut,
			size_t& index) const
		{
			JD_OBJECT_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_11);
			for (size_t i = 0; i < locks.size(); ++i)
			{
				const ObjectLockData& lock = locks[i];
				if (lock.data.objectID == targetID)
				{
					lockOut = lock;
					index = i;
					return true;
				}
			}
			return false;
		}
		void JDObjectLocker::getObjectLockDataFromSessionID(const std::vector<ObjectLockData>& locks,
			const std::string& targetSessionID,
			const std::string& userName,
			std::vector<ObjectLockData>& locksOut,
			std::vector<size_t>& matches,
			std::vector<size_t>& mismatches) const
		{
			JD_OBJECT_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_11);
			for (size_t i = 0; i < locks.size(); ++i)
			{
				const ObjectLockData& lock = locks[i];
				if (lock.data.sessionID == targetSessionID && lock.data.owner == userName)
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