#include "manager/JDManager.h"
#include "manager/JDObjectLocker.h"
#include "utilities/filesystem/FileLock.h"
#include "utilities/JDUniqueMutexLock.h"
#include "utilities/JDUtilities.h"


#include <QFile>
#ifdef JD_USE_QJSON
#include <QJsonDocument>
#include <QJsonArray>
#else
#include "Json/JsonSerializer.h"
#include "Json/JsonDeserializer.h"
#endif
#include <QDateTime>

namespace JsonDatabase
{
	namespace Internal
	{
		// Helper macros
#define FILE_LOCK(errRetVal) \
	FileLock fileLock(getTablePath(), getTableFileName()); \
	{ \
		FileLock::Error lockErr; \
		if (!fileLock.lock(m_lockTableTryGetLockTimeoutMs, lockErr)) \
		{ \
			err = Error::unableToLock; \
			JD_CONSOLE_FUNCTION(" Can't aquire lock for file: " << fileLock.getFilePath() \
				<< " timeout occured after: " << m_lockTableTryGetLockTimeoutMs << "ms" \
				<< " LockError: " << FileLock::getErrorStr(lockErr) << "\n"); \
			return errRetVal; \
		} \
	} 



		JDObjectLocker::JDObjectLocker(JDManager& manager, std::mutex& mtx)
			: m_manager(manager)
			, m_mutex(mtx)
			, m_lockTableFile("lockTable.json")
			, m_lockTableTryGetLockTimeoutMs(1000)
			, m_useSpecificDatabasePath(false)
			, m_specificDatabasePath("")
		{

		}

		JDObjectLocker::~JDObjectLocker()
		{
			m_lockTableWatcher.stop();
			Error err;
			unlockAllObjs(err);
		}
		bool JDObjectLocker::setup(Error &err)
		{
			JDM_UNIQUE_LOCK_P;
			JD_OBJECT_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_10);

			bool success = false;
			err = Error::none;
			QFile file(getTableFileFilePath().c_str());
			if (!file.exists())
			{
				FILE_LOCK(false); // Try's to lok the file, if it fails it returns
				writeLockTable({}, err);					
			}
			if (!file.exists())
				success = false;

			m_lockTableWatcher.setup(getTableFileFilePath());
			return success;
		}

		bool JDObjectLocker::lockObject(const JDObject & obj, Error& err) const
		{
			if (!obj.get())
			{
				err = Error::objIsNullptr;
				return false;
			}
			JDM_UNIQUE_LOCK_P;
			JD_OBJECT_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_10);

			err = Error::none;
			FILE_LOCK(false); // Try's to lok the file, if it fails it returns

			std::vector<ObjectLockData> locks;
			if (!readLockTable(locks, err))
				return false;

			// Check if Lock is already aquired 
			ObjectLockData targetLock;
			size_t targetIndex;
			Utilities::JDUser user = m_manager.getUser();
			if (getObjectLockDataFromID(locks, obj->getObjectID(), targetLock, targetIndex))
			{
				
				if (targetLock.data.user.getSessionID() == user.getSessionID())
				{
					// Already locked by this session
					JD_CONSOLE("bool JDObjectLocker::lockObject(obj:\"" 
						<< obj->getObjectID()->toString() << "\") Lock for object: \"" 
						+ obj->getObjectID()->toString() + "\" type: \"" + obj->className() 
						+ "\" is already aquired in this session\n");
					err = Error::none;
					return true;
				}
				else
				{
					err = Error::lockedByOther;
					JD_CONSOLE("bool JDObjectLocker::lockObject(obj:\"" 
						<< obj->getObjectID()->toString() << "\") Can't aquire lock for object: \"" 
						+ obj->getObjectID()->toString() + "\" type: \"" + obj->className() +
						"\"\nLock Data: \n" + targetLock.toString() + "\n"
						"Lock is already aquired from user: \"" + targetLock.data.user.toString() + "\"\n");
					return false;
				}
			}

			targetLock.setObject(obj, m_manager);
			

			locks.push_back(targetLock);
			if (!writeLockTable(locks, err))
				return false;


			// Verify lock success
			{
				JD_OBJECT_LOCK_PROFILING_BLOCK("Verify object lock", JD_COLOR_STAGE_11);
				std::vector<ObjectLockData> verifyLocks;
				if (!readLockTable(verifyLocks, err))
					return false;

				// Check if Lock is already aquired 
				ObjectLockData targetLock2;
				size_t targetIndex2;
				if (getObjectLockDataFromID(locks, obj->getObjectID(), targetLock2, targetIndex2))
				{
					if (targetLock2.data.user.getSessionID() == user.getSessionID())
					{
						// Already locked by this session
						err = Error::none;
						return true;
					}
					else
					{
						err = Error::lockedByOther;
						JD_CONSOLE("Can't verify the locked object: " 
							<< obj->getObjectID() 
							<<" Object is locked by: \""<< targetLock2.data.user.toString() <<"\"");
						return false;
					}
				}
				else
				{
					err = Error::unableToLock;
					JD_CONSOLE("Can't verify the locked object: "<< obj->getObjectID());
					return false;
				}
			}

			err = Error::none;
			return true;
		}
		bool JDObjectLocker::unlockObject(const JDObject & obj, Error& err) const
		{
			if (!obj.get())
			{
				err = Error::objIsNullptr;
				return false;
			}
			JDM_UNIQUE_LOCK_P;
			JD_OBJECT_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_10);

			err = Error::none;
			FILE_LOCK(false); // Try's to lok the file, if it fails it returns

			std::vector<ObjectLockData> locks;
			if (!readLockTable(locks, err))
				return false;

			// Check if Lock is already aquired 
			ObjectLockData targetLock;
			size_t targetIndex;
			Utilities::JDUser user = m_manager.getUser();
			if (getObjectLockDataFromID(locks, obj->getObjectID(), targetLock, targetIndex))
			{
				if (targetLock.data.user.getSessionID() == user.getSessionID())
				{
					// Locked by this session

					// Remove lock from list
					locks.erase(locks.begin() + targetIndex);

					// Save table
					if (!writeLockTable(locks, err))
						return false;
					return true;
				}
				else
				{
					err = Error::lockedByOther;
					
					JD_CONSOLE_FUNCTION("Can't release lock for object: \"" 
						<< obj->getObjectID()->toString() << "\" type: \"" << obj->className()
						<< "\"\nLock Data: \n" + targetLock.toString() + "\n"
						<< "Lock is owned by user: " + targetLock.data.user.toString() + "\n");
					return false;
				}
			}
			JD_CONSOLE_FUNCTION("Lock for object: \"" + 
				obj->getObjectID()->toString() + "\" type: \"" + 
				obj->className() + "\" did not exist\n");

			err = Error::none;
			return true;
		}
		bool JDObjectLocker::unlockAllObjs(Error& err) const
		{
			JDM_UNIQUE_LOCK_P;
			JD_OBJECT_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_10);
			
			err = Error::none;
			FILE_LOCK(false); // Try's to lok the file, if it fails it returns

			std::vector<ObjectLockData> locks;
			if (!readLockTable(locks, err))
				return false;

			std::vector<ObjectLockData> locksOut;
			std::vector<size_t> matches;
			std::vector<size_t> mismatches;
			getObjectLockDataFromSessionID(locks, m_manager.getSessionID(), locksOut, matches, mismatches);

			// Save all locks which do not match, with this session id, to a new table
			std::vector<ObjectLockData> newLockTable;
			for (size_t i = 0; i < mismatches.size(); ++i)
			{
				newLockTable.push_back(locks[mismatches[i]]);
			}

			if (!writeLockTable(newLockTable, err))
				return false;

			err = Error::none;
			return true;
		}
		bool JDObjectLocker::isObjectLocked(const JDObject & obj, Error& err) const
		{
			if (!obj.get())
			{
				err = Error::objIsNullptr;
				return false;
			}
			JDM_UNIQUE_LOCK_P;
			JD_OBJECT_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_5);

			err = Error::none;
			FILE_LOCK(false); // Try's to lok the file, if it fails it returns

			std::vector<ObjectLockData> locks;
			if (!readLockTable(locks, err))
			{
				return false;
			}
			// Check if Lock is already aquired 
			ObjectLockData targetLock;
			size_t targetIndex;
			if (getObjectLockDataFromID(locks, obj->getObjectID(), targetLock, targetIndex))
			{
				return true; // Object is locked by any user
			}
			return false;
		}
		bool JDObjectLocker::isObjectLockedByMe(const JDObject & obj, Error& err) const
		{
			if (!obj.get())
			{
				err = Error::objIsNullptr;
				return false;
			}
			JDM_UNIQUE_LOCK_P;
			JD_OBJECT_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_5);

			err = Error::none;
			FILE_LOCK(false); // Try's to lok the file, if it fails it returns

			std::vector<ObjectLockData> locks;
			if (!readLockTable(locks, err))
			{
				return false;
			}

			// Check if Lock is already aquired 
			ObjectLockData targetLock;
			size_t targetIndex;
			if (getObjectLockDataFromID(locks, obj->getObjectID(), targetLock, targetIndex))
			{
				if (targetLock.data.user.getSessionID() == m_manager.getUser().getSessionID())
				{
					// Already locked by this session
					return true;
				}
			}
			return false;
		}
		bool JDObjectLocker::isObjectLockedByOther(const JDObject & obj, Error& err) const
		{
			if (!obj.get())
			{
				err = Error::objIsNullptr;
				return false;
			}
			JDM_UNIQUE_LOCK_P;
			JD_OBJECT_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_5);

			err = Error::none;
			FILE_LOCK(false); // Try's to lok the file, if it fails it returns

			std::vector<ObjectLockData> locks;
			if (!readLockTable(locks, err))
			{
				return false;
			}

			// Check if Lock is already aquired 
			ObjectLockData targetLock;
			size_t targetIndex;
			if (getObjectLockDataFromID(locks, obj->getObjectID(), targetLock, targetIndex))
			{
				if (targetLock.data.user.getSessionID() == m_manager.getUser().getSessionID())
				{
					// Already locked by this session
					return true;
				}
			}
			return false;
		}
		bool JDObjectLocker::getLockedObjects(std::vector<LockData>& lockedObjectsOut, Error& err) const
		{
			JDM_UNIQUE_LOCK_P;
			JD_OBJECT_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_5);
			lockedObjectsOut.clear();

			err = Error::none;
			FILE_LOCK(false); // Try's to lok the file, if it fails it returns

			std::vector<ObjectLockData> locks;
			if (!readLockTable(locks, err))
			{
				return false;
			}

			lockedObjectsOut.reserve(locks.size());
			bool success = true;
			for (const ObjectLockData& lock : locks)
			{
				if (lock.data.objectID != JDObjectID::invalidID)
					lockedObjectsOut.push_back(lock.data);
				else
				{
					JD_CONSOLE_FUNCTION("Object has empty objectID: "
						<< lock.toString() << "\n");
					err = corruptTableData;
					success = false;
				}
			}
			
			return success;
		}



		const std::string& JDObjectLocker::getErrorStr(Error err) const
		{
			switch (err)
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
			unknown = "Unknown JDObjectLocker Error: " + std::to_string(err);
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
		void JDObjectLocker::onDatabasePathChange(const std::string& oldPath, const std::string& newPath, Error& err) const
		{
			JD_OBJECT_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_10);

			FileLock::Error lockErr1;
			FileLock fileLock1(oldPath, m_lockTableFile);
			if (!fileLock1.lock(m_lockTableTryGetLockTimeoutMs, lockErr1))
			{
				err = Error::unableToLock;
				JD_CONSOLE_FUNCTION("Can't aquire lock for file: " << fileLock1.getFilePath()
					<< " timeout occured after: " << m_lockTableTryGetLockTimeoutMs << "ms"
					<< " LockError: " << fileLock1.getErrorStr(lockErr1) << "\n");
				return;
			}
			FileLock::Error lockErr2;
			FileLock fileLock2(newPath, m_lockTableFile);
			if (!fileLock2.lock(m_lockTableTryGetLockTimeoutMs, lockErr2))
			{
				err = Error::unableToLock;
				JD_CONSOLE_FUNCTION("Can't aquire lock for file: " << fileLock2.getFilePath()
					<< " timeout occured after: " << m_lockTableTryGetLockTimeoutMs << "ms"
					<< " LockError: " << fileLock2.getErrorStr(lockErr2) << "\n");
				return;
			}
			m_lockTableWatcher.stop();

			m_useSpecificDatabasePath = true;
			m_specificDatabasePath = oldPath;

			std::vector<ObjectLockData> locks;
			if (!readLockTable(locks, err))
			{
				m_useSpecificDatabasePath = false;
				return;
			}

			std::vector<ObjectLockData> locksOut;
			std::vector<size_t> matches;
			std::vector<size_t> mismatches;
			getObjectLockDataFromSessionID(locks, m_manager.getSessionID(), locksOut, matches, mismatches);

			std::vector<ObjectLockData> locksFromOldLocation;
			for (size_t i = 0; i < mismatches.size(); ++i)
			{
				locksFromOldLocation.push_back(locks[mismatches[i]]);
			}
			writeLockTable(locksFromOldLocation, err);


			m_specificDatabasePath = newPath;
			std::vector<ObjectLockData> locksFromNewLocation;

			if (!readLockTable(locksFromNewLocation, err))
			{
				m_useSpecificDatabasePath = false;
				m_lockTableWatcher.setup(getTableFileFilePath());
				return;
			}

			for (size_t i = 0; i < matches.size(); ++i)
			{
				locksFromNewLocation.push_back(locks[matches[i]]);
			}

			if (!writeLockTable(locksFromNewLocation, err))
			{
				m_useSpecificDatabasePath = false;
				m_lockTableWatcher.setup(getTableFileFilePath());
				return;
			}

			m_useSpecificDatabasePath = false;
			err = Error::none;
			m_lockTableWatcher.setup(getTableFileFilePath());
			return;
		}


		JDObjectLocker::ObjectLockData::ObjectLockData()
			: obj(nullptr)
		{
			
		}
		JDObjectLocker::ObjectLockData::ObjectLockData(const JDObject & obj, const JDManager& manager)
			: obj(nullptr)
		{
			setObject(obj, manager);
		}
		void JDObjectLocker::ObjectLockData::setObject(const JDObject & obj, const JDManager& manager)
		{
			this->obj = obj;
			if (!this->obj.get())
				return;
			data.objectID  = this->obj->getObjectID()->get();
			data.user      = manager.getUser();
			data.lockDate  = QDate::currentDate();
			data.lockTime  = QTime::currentTime();
		}
#ifdef JD_USE_QJSON
		bool JDObjectLocker::ObjectLockData::load(const QJsonObject& obj)
#else
		bool JDObjectLocker::ObjectLockData::load(const JsonObject& obj)
#endif
		{
			if(!isValid(obj))
				return false;
#ifdef JD_USE_QJSON
			data.objectID = obj[JsonKeys::objectID.data()].toInt();
			data.user.load(obj[JsonKeys::user.data()].toObject());
			data.lockDate = Utilities::stringToQDate(obj[JsonKeys::lockDate.data()].toString().toStdString());
			data.lockTime = Utilities::stringToQTime(obj[JsonKeys::lockTime.data()].toString().toStdString());
#else

			data.objectID = obj.at(JsonKeys::objectID).getInt();
			data.user.load(obj.at(JsonKeys::user).getObject());
			data.lockDate = Utilities::stringToQDate(obj.at(JsonKeys::lockDate).toString());
			data.lockTime = Utilities::stringToQTime(obj.at(JsonKeys::lockTime).toString());
#endif
			return true;
		}
#ifdef JD_USE_QJSON
		bool JDObjectLocker::ObjectLockData::save(QJsonObject& obj) const
#else
		bool JDObjectLocker::ObjectLockData::save(JsonObject& obj) const
#endif
		{
			JD_OBJECT_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_11);
			bool success = true;
#ifdef JD_USE_QJSON
			obj[JsonKeys::objectID.data()] = data.objectID;
			QJsonObject userObj;
			success &= data.user.save(userObj);
			obj[JsonKeys::user.data()] = userObj;
			obj[JsonKeys::lockDate.data()] = Utilities::qDateToString(data.lockDate).c_str();
			obj[JsonKeys::lockTime.data()] = Utilities::qTimeToString(data.lockTime).c_str();
#else
			obj.reserve(5);
			obj[JsonKeys::objectID] = data.objectID;
			JsonObject userObj;
			success &= data.user.save(userObj);
			obj[JsonKeys::user] = userObj;
			obj[JsonKeys::lockDate] = Utilities::qDateToString(data.lockDate);
			obj[JsonKeys::lockTime] = Utilities::qTimeToString(data.lockTime);
#endif
			return true;
		}

#ifdef JD_USE_QJSON
		bool JDObjectLocker::ObjectLockData::isValid(const QJsonObject& lock)
#else
		bool JDObjectLocker::ObjectLockData::isValid(const JsonObject& lock)
#endif
		{
			JD_OBJECT_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_11);
			if (!lock.contains(JsonKeys::objectID.data())) return false;
			if (!lock.contains(JsonKeys::user.data())) return false;
			if (!lock.contains(JsonKeys::lockDate.data())) return false;
			if (!lock.contains(JsonKeys::lockTime.data())) return false;

			return true;
		}
		std::string JDObjectLocker::ObjectLockData::toString() const
		{
#ifdef JD_USE_QJSON
			QJsonObject obj;
#else
			JsonObject obj;
#endif
			save(obj);
#ifdef JD_USE_QJSON
			QJsonDocument jsonDoc(obj);

			// Convert the JSON document to a string
			QString jsonString = jsonDoc.toJson(QJsonDocument::Indented);
			return jsonString.toStdString();
#else
			JsonSerializer serializer;
			return serializer.serializeObject(obj);
#endif
		}


		
		
		

		bool JDObjectLocker::readLockTable(std::vector<ObjectLockData>& locks, Error& err) const
		{
			JD_OBJECT_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_11);
			std::string filePath = getTableFileFilePath();
			QFile file(filePath.c_str());
			if (!file.exists())
			{
				JD_CONSOLE_FUNCTION("File: " << filePath.c_str() << " does not exist\n");
				//err = Error::tableFileNotExist;
				if (file.open(QIODevice::WriteOnly))
					file.close();
				return true;
			}
			if (!file.open(QIODevice::ReadOnly)) {
				JD_CONSOLE_FUNCTION("Failed to open file for reading: " << filePath.c_str() << "\n");
				err = Error::cantOpenTableFile;
				return false;
			}

			std::string jsonData = file.readAll().constData();
#ifdef JD_USE_QJSON
			QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData.c_str());
			if (!jsonDoc.isNull()) {
				if (!jsonDoc.isArray())
				{
					JD_CONSOLE_FUNCTION("Tabledata: " << jsonData.c_str() << " is not a QJsonArray\n");
					err = Error::corruptTableData;
					return false;
				}
				QJsonArray array = jsonDoc.array();
				for (const QJsonValue& value : array)
				{
					if (!value.isObject())
					{
						JD_CONSOLE_FUNCTION("Tabledata value: " << value.toString().toStdString().c_str() << " is not a QJsonObject\n");
						err = Error::corruptTableData;
						return false;
					}
					QJsonObject lock = value.toObject();
					if (!ObjectLockData::isValid(lock))
					{
						JD_CONSOLE_FUNCTION("Tabledata value: " << value.toString().toStdString().c_str() << " is has missing or corrupt data\n");
						err = Error::corruptTableData;
						return false;
					}

					locks.push_back(ObjectLockData());
					locks[locks.size()-1].load(lock);
				}
			}
#else
			JsonDeserializer deserializer;
			JsonValue deserialized = deserializer.deserializeValue(jsonData);
			if (!deserialized.isNull()) {
				if (!deserialized.isArray())
				{
					JD_CONSOLE_FUNCTION("Tabledata: " << jsonData.c_str() << " is not a JsonArray\n");
					err = Error::corruptTableData;
					return false;
				}

				JsonArray array = deserialized.getArray();
				for (const JsonValue& value : array)
				{
					if (!value.isObject())
					{
						JD_CONSOLE_FUNCTION("Tabledata value: " << value.toString() << " is not a JsonObject\n");
						err = Error::corruptTableData;
						return false;
					}
					JsonObject lock = value.getObject();
					if (!ObjectLockData::isValid(lock))
					{
						JD_CONSOLE_FUNCTION("Tabledata value: " << value.toString() << " is has missing or corrupt data\n");
						err = Error::corruptTableData;
						return false;
					}

					locks.push_back(ObjectLockData());
					locks[locks.size() - 1].load(lock);
				}
			}
#endif
			return true;
		}
		bool JDObjectLocker::writeLockTable(const std::vector<ObjectLockData>& locks, Error& err) const
		{
			JD_OBJECT_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_11);
			std::string filePath = getTableFileFilePath();
			
			QFile file(filePath.c_str());
			if (!file.open(QIODevice::WriteOnly)) {
				JD_CONSOLE_FUNCTION("Failed to open file for writing: " << filePath.c_str() << "\n");
				err = Error::cantOpenTableFile;
				return false;
			}
#ifdef JD_USE_QJSON
			QJsonArray array;
#else
			JsonArray array;
			array.reserve(locks.size());
#endif
			for (const ObjectLockData& lock : locks)
			{
#ifdef JD_USE_QJSON
				QJsonObject sav;
				lock.save(sav);
#else
				JsonObject sav;
				lock.save(sav);
#endif
				if (!ObjectLockData::isValid(sav))
				{
					err = Error::programmingError;
					JD_CONSOLE_FUNCTION("Programming error!!! ajust the ObjectLockData::isValid(const QJsonObject&) function\n");
					return false;
				}
#ifdef JD_USE_QJSON
				array.append(sav);
#else
				array.emplace_back(sav);
#endif
			}

			QByteArray transmittStr;
#ifdef JD_USE_QJSON
			QJsonDocument jsonDoc(array);
			transmittStr = jsonDoc.toJson(QJsonDocument::Indented);
#else
			JsonSerializer serializer;
			transmittStr = QByteArray::fromStdString(serializer.serializeArray(array));
#endif

			//m_lockTableWatcher.pause();

			file.write(transmittStr);
			file.close();

			//m_lockTableWatcher.unpause();
			return true;
		}
		std::string JDObjectLocker::getTablePath() const
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
			const JDObjectIDptr& targetID,
			ObjectLockData& lockOut,
			size_t& index) const
		{
			JD_OBJECT_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_11);
			for (size_t i = 0; i < locks.size(); ++i)
			{
				const ObjectLockData& lock = locks[i];
				if (lock.data.objectID == targetID->get())
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
			//const Utilities::JDUser& user,
			std::vector<ObjectLockData>& locksOut,
			std::vector<size_t>& matches,
			std::vector<size_t>& mismatches) const
		{
			JD_OBJECT_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_11);
			for (size_t i = 0; i < locks.size(); ++i)
			{
				const ObjectLockData& lock = locks[i];
				if (lock.data.user.getSessionID() == targetSessionID)
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