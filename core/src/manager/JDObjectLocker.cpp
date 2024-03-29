#include "manager/JDManager.h"
#include "manager/JDObjectLocker.h"
#include "utilities/filesystem/FileLock.h"
#include "utilities/JDUniqueMutexLock.h"
#include "utilities/JDUtilities.h"


#include <QFile>

#include "Json/JsonSerializer.h"
#include "Json/JsonDeserializer.h"

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

		const std::string JDObjectLocker::JsonKeys::objectID = "objID";
		const std::string JDObjectLocker::JsonKeys::user = "user";
		const std::string JDObjectLocker::JsonKeys::lockDate = "lockDate";
		const std::string JDObjectLocker::JsonKeys::lockTime = "lockTime";

		JDObjectLocker::JDObjectLocker(JDManager& manager, std::mutex& mtx)
			: AbstractRegistry()
			, m_manager(manager)
			, m_mutex(mtx)
			, m_lockTableFile("lockTable.json")
			, m_registryOpenTimeoutMs(1000)
			, m_useSpecificDatabasePath(false)
			, m_specificDatabasePath("")
		{
			AbstractRegistry::setName("lockedObjects");
		}

		JDObjectLocker::~JDObjectLocker()
		{
			m_lockTableWatcher.stop();
			Error err;
			unlockAllObjs(err);
		}
		/*bool JDObjectLocker::setup(Error& err)
		{
			JDM_UNIQUE_LOCK_P;
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_10);

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
		}*/

		bool JDObjectLocker::lockObject(const JDObject & obj, Error& err)
		{
			if (!obj.get())
			{
				err = Error::objIsNullptr;
				return false;
			}
			JDM_UNIQUE_LOCK_P;
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_10);

			if (!AbstractRegistry::openRegistryFile(m_registryOpenTimeoutMs))
				return false;
			AbstractRegistry::AutoClose autoClose(this);
			AbstractRegistry::removeInactiveObjects();

			std::vector<std::shared_ptr<LockEntryObjectImpl>> loadedObjects;
			if (!AbstractRegistry::readObjects<LockEntryObjectImpl>(loadedObjects))
			{
				err = Error::corruptTableData;
				return false;
			}

			// Check if Lock is already aquired
			Utilities::JDUser user = m_manager.getUser();
			for (size_t i = 0; i < loadedObjects.size(); ++i)
			{
				if (loadedObjects[i]->data.objectID == obj->getObjectID()->get())
				{
					if (loadedObjects[i]->data.user.getSessionID() == user.getSessionID())
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
							"\"\nLock Data: \n" + loadedObjects[i]->toString() + "\n"
							"Lock is already aquired from user: \"" + loadedObjects[i]->data.user.toString() + "\"\n");
						return false;
					}
				}
			}

			auto newEntry = std::make_shared<LockEntryObjectImpl>(obj->getObjectID()->toString(), obj, m_manager);
			int ret = 0;
			
			if ((ret = AbstractRegistry::addObjects({ newEntry })) != 1)
			{
				err = Error::unableToLock;
				JD_CONSOLE("Can't lock object: " << obj->getObjectID());
				return false;
			}
			err = Error::none;
			return true;

			/*err = Error::none;
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
				JD_REGISTRY_PROFILING_BLOCK("Verify object lock", JD_COLOR_STAGE_11);
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
			return true;*/
		}
		bool JDObjectLocker::unlockObject(const JDObject & obj, Error& err)
		{
			if (!obj.get())
			{
				err = Error::objIsNullptr;
				return false;
			}
			JDM_UNIQUE_LOCK_P;
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_10);

			if (!AbstractRegistry::openRegistryFile(m_registryOpenTimeoutMs))
				return false;
			AbstractRegistry::AutoClose autoClose(this);

			std::string key = obj->getObjectID()->toString();

			if(!AbstractRegistry::lockExists(key))
			{
				err = Error::none;
				JD_CONSOLE_FUNCTION("Lock for object: \"" +
					obj->getObjectID()->toString() + "\" type: \"" +
					obj->className() + "\" did not exist\n");
				return false;
			}

			if (!AbstractRegistry::isSelfOwned(key))
			{
				err = Error::lockedByOther;
				JD_CONSOLE_FUNCTION("Can't release lock for object: \"" 
					<< obj->getObjectID()->toString() << "\" type: \"" << obj->className()
					<< "\"\nLock is owned by another session\n");
				return false;
			}

			int removed = 0;
			if ((removed = AbstractRegistry::removeObjects({ key })) != 1)
			{
				JD_CONSOLE_FUNCTION("Can't release lock for object: \""
					<< obj->getObjectID()->toString() << "\" type: \"" << obj->className()
					<< "\"\n");
				return false;
			}
			return true;
		}
		bool JDObjectLocker::unlockAllObjs(Error& err)
		{
			JDM_UNIQUE_LOCK_P;
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_10);
			err = Error::none;
			if (!AbstractRegistry::openRegistryFile(m_registryOpenTimeoutMs))
				return false;
			AbstractRegistry::AutoClose autoClose(this);
			if (!AbstractRegistry::removeAllSelfOwnedObjects())
			{
				JD_CONSOLE_FUNCTION("Can't release all locked objects\n");
				return false;
			}
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
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_5);


			if (!AbstractRegistry::openRegistryFile(m_registryOpenTimeoutMs))
				return false;
			AbstractRegistry::AutoClose autoClose(this);
			return AbstractRegistry::isObjectActive(obj->getObjectID()->toString());
		}
		bool JDObjectLocker::isObjectLockedByMe(const JDObject & obj, Error& err) const
		{
			if (!obj.get())
			{
				err = Error::objIsNullptr;
				return false;
			}
			JDM_UNIQUE_LOCK_P;
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_5);

			if (!AbstractRegistry::openRegistryFile(m_registryOpenTimeoutMs))
				return false;
			AbstractRegistry::AutoClose autoClose(this);

			if(AbstractRegistry::isSelfOwned(obj->getObjectID()->toString()))
				return true;

			return false;
		}
		bool JDObjectLocker::isObjectLockedByOther(const JDObject & obj, Error& err) const
		{
			err = Error::none;
			if (!obj.get())
			{
				err = Error::objIsNullptr;
				return false;
			}
			JDM_UNIQUE_LOCK_P;
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_5);

			if (!AbstractRegistry::openRegistryFile(m_registryOpenTimeoutMs))
				return false;
			AbstractRegistry::AutoClose autoClose(this);

			std::string key = obj->getObjectID()->toString();

			if(AbstractRegistry::lockExists(key))
			{
				if (!AbstractRegistry::isSelfOwned(key))
					return true;
			}
			return false;
		}

		
		bool JDObjectLocker::getLockedObjects(std::vector<LockData>& lockedObjectsOut, Error& err) const
		{
			JDM_UNIQUE_LOCK_P;
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_5);
			lockedObjectsOut.clear();

			if (!AbstractRegistry::openRegistryFile(m_registryOpenTimeoutMs))
				return false;
			AbstractRegistry::AutoClose autoClose(this);

			std::vector<std::shared_ptr<LockEntryObjectImpl>> loadedObjects;
			if (!AbstractRegistry::readObjects<LockEntryObjectImpl>(loadedObjects))
			{
				err = Error::corruptTableData;
				return false;
			}
			bool success = true;
			for(auto obj : loadedObjects)
			{
				if(obj->data.objectID != JDObjectID::invalidID)
					lockedObjectsOut.push_back(obj->data);
				else
				{
					JD_CONSOLE_FUNCTION("Object has empty objectID: "
						<< obj->toString() << "\n");
					err = corruptTableData;
					success = false;
				}
			}
			return success;
		}
		int JDObjectLocker::removeInactiveObjectLocks() const
		{
			if (!AbstractRegistry::openRegistryFile(m_registryOpenTimeoutMs))
				return 0;
			AbstractRegistry::AutoClose autoClose(this);
			return AbstractRegistry::removeInactiveObjects();
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
			if (m_lockTableWatcher.hasFileChanged())
			{
				m_lockTableWatcher.clearHasFileChanged();
				m_manager.onObjectLockerFileChanged();
				m_manager.getSignals().lockedObjectsChanged.emitSignal();
			}
		}

		void JDObjectLocker::onCreateFiles()
		{
			m_lockTableWatcher.setup(AbstractRegistry::getRegistrationFilePath());
		}

		void JDObjectLocker::onDatabasePathChangeStart(const std::string& newPath)
		{
			JD_UNUSED(newPath);
		}
		void JDObjectLocker::onDatabasePathChangeEnd()
		{

		}

		void JDObjectLocker::onNameChange(const std::string& newName)
		{
			JD_UNUSED(newName);
		}


		JDObjectLocker::LockEntryObjectImpl::LockEntryObjectImpl(const std::string& key)
			: LockEntryObject(key)
			, obj(nullptr)
		{
			
		}
		JDObjectLocker::LockEntryObjectImpl::LockEntryObjectImpl(const std::string& key, const JDObject & obj, const JDManager& manager)
			: LockEntryObject(key)
			, obj(nullptr)
		{
			setObject(obj, manager);
		}
		JDObjectLocker::LockEntryObjectImpl::~LockEntryObjectImpl()
		{

		}
		void JDObjectLocker::LockEntryObjectImpl::setObject(const JDObject & obj, const JDManager& manager)
		{
			this->obj = obj;
			if (!this->obj.get())
				return;
			data.objectID  = this->obj->getObjectID()->get();
			data.user      = manager.getUser();
			data.lockDate  = QDate::currentDate();
			data.lockTime  = QTime::currentTime();
		}
		bool JDObjectLocker::LockEntryObjectImpl::load(const JsonObject& obj)
		{
			bool success = LockEntryObject::load(obj);
			if(!isValid(obj))
				return false;
			data.objectID = obj.at(JsonKeys::objectID).get<JDObjectID::IDType>();
			data.user.load(obj.at(JsonKeys::user).get<JsonObject>());
			
			data.lockDate = Utilities::stringToQDate(obj.at(JsonKeys::lockDate).get<std::string>());
			data.lockTime = Utilities::stringToQTime(obj.at(JsonKeys::lockTime).get<std::string>());

			return success;
		}

		bool JDObjectLocker::LockEntryObjectImpl::save(JsonObject& obj) const
		{
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_11);
			bool success = LockEntryObject::save(obj);

			obj[JsonKeys::objectID] = data.objectID;
			JsonObject userObj;
			success &= data.user.save(userObj);
			obj[JsonKeys::user] = userObj;
			obj[JsonKeys::lockDate] = Utilities::qDateToString(data.lockDate);
			obj[JsonKeys::lockTime] = Utilities::qTimeToString(data.lockTime);
			return true;
		}

		bool JDObjectLocker::LockEntryObjectImpl::isValid(const JsonObject& lock)
		{
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_11);
			if (!lock.contains(JsonKeys::objectID.data())) return false;
			if (!lock.contains(JsonKeys::user.data())) return false;
			if (!lock.contains(JsonKeys::lockDate.data())) return false;
			if (!lock.contains(JsonKeys::lockTime.data())) return false;

			return true;
		}
		std::string JDObjectLocker::LockEntryObjectImpl::toString() const
		{
			JsonObject obj;
			save(obj);

			JsonSerializer serializer;
			return serializer.serializeObject(obj);
		}


		
		
		
	}

	
}