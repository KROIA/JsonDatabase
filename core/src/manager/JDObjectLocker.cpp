#include "manager/JDManager.h"
#include "manager/JDObjectLocker.h"
#include "utilities/filesystem/FileLock.h"
#include "utilities/JDUniqueMutexLock.h"
#include "utilities/JDUtilities.h"
#include "utilities/JDUser.h"


#include <QFile>

#include "Json/JsonSerializer.h"
#include "Json/JsonDeserializer.h"

#include <QDateTime>

namespace JsonDatabase
{
	namespace Internal
	{
		const std::string JDObjectLocker::JsonKeys::objectID = "objID";
		const std::string JDObjectLocker::JsonKeys::user = "user";
		const std::string JDObjectLocker::JsonKeys::lockDate = "lockDate";
		const std::string JDObjectLocker::JsonKeys::lockTime = "lockTime";

		JDObjectLocker::JDObjectLocker(JDManager& manager)
			: AbstractRegistry()
			, m_manager(manager)
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
			delete m_logger;
		}
	

		bool JDObjectLocker::lockObject(const JDObject& obj, Error& err)
		{
			std::vector<JDObject> objs = { obj };
			std::vector<Error> errors;
			if (!lockObjects_internal(objs, errors))
			{
				err = errors[0];
				return false;
			}
			err = errors[0];
			return true;
		}

		bool JDObjectLocker::lockObjects(const std::vector<JDObject>& objs, std::vector<Error>& errors)
		{
			return lockObjects_internal(objs, errors);
		}

		bool JDObjectLocker::unlockObject(const JDObject& obj, Error& err)
		{
			std::vector<JDObject> objs = { obj };
			std::vector<Error> errors;
			if (!unlockObjects_internal(objs, errors))
			{
				err = errors[0];
				return false;
			}
			err = errors[0];
			return true;
		}
		bool JDObjectLocker::unlockObject(const JDObjectID::IDType& id, Error& err)
		{
			JDObject obj = m_manager.getObject_internal(id);
			std::vector<JDObject> objs = { obj };
			std::vector<Error> errors;
			if (!unlockObjects_internal(objs, errors))
			{
				err = errors[0];
				return false;
			}
			err = errors[0];
			return true;
		}

		bool JDObjectLocker::unlockObjects(const std::vector<JDObject>& objs, std::vector<Error>& errors)
		{
			return unlockObjects_internal(objs, errors);
		}
		bool JDObjectLocker::unlockObjects(const std::vector<JDObjectID::IDType>& objs, std::vector<Error>& errors)
		{
			std::vector<JDObject> objects;
			objects.reserve(objs.size());
			for (auto id : objs)
			{
				objects.push_back(m_manager.getObject_internal(id));
			}
			return unlockObjects_internal(objects, errors);
		}

		bool JDObjectLocker::lockAllObjs(Error& err)
		{
			std::vector<Error> errors;
			std::vector<JDObject> objs = m_manager.getObjects_internal();
			if (!lockObjects_internal(objs, errors))
			{
				err = Error::unableToLockObject;
				return false;
			}
			err = Error::none;
			return true;
		}

		bool JDObjectLocker::unlockAllObjs(Error& err)
		{
			std::vector<Error> errors;
			std::vector<JDObjectLocker::LockData> locks;
			Error err_;
			std::vector<JDObject> lockedObjs;
			if (!m_manager.getLockedObjects(lockedObjs,err_))
			{
				lockedObjs = m_manager.getObjects_internal();
			}
			if (!unlockObjects_internal(lockedObjs, errors))
			{
				err = Error::unableToUnlockObject;
				return false;
			}
			err = Error::none;
			return true;
		}
		bool JDObjectLocker::isObjectLocked(const JDObject& obj, Error& err) const
		{
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_5);
			err = Error::none;
			if (!obj.get())
			{
				err = Error::objIsNullptr;
				return false;
			}
			if (!AbstractRegistry::openRegistryFile(m_registryOpenTimeoutMs))
			{
				err = Error::cantOpenRegistryFile;
				return false;
			}
			AbstractRegistry::AutoClose autoClose(this);
			return AbstractRegistry::isObjectActive(obj->getObjectID()->toString());
		}
		bool JDObjectLocker::isObjectLockedByMe(const JDObject& obj, Error& err) const
		{
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_5);
			err = Error::none;
			if (!obj.get())
			{
				err = Error::objIsNullptr;
				return false;
			}			
			if (!AbstractRegistry::openRegistryFile(m_registryOpenTimeoutMs))
			{
				err = Error::cantOpenRegistryFile;
				return false;
			}
			AbstractRegistry::AutoClose autoClose(this);

			if (AbstractRegistry::isSelfOwned(obj->getObjectID()->toString()))
				return true;

			return false;
		}
		bool JDObjectLocker::isObjectLockedByOther(const JDObject& obj, Error& err) const
		{
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_5);
			err = Error::none;
			if (!obj.get())
			{
				err = Error::objIsNullptr;
				return false;
			}
			if (!AbstractRegistry::openRegistryFile(m_registryOpenTimeoutMs))
			{
				err = Error::cantOpenRegistryFile;
				return false;
			}
			AbstractRegistry::AutoClose autoClose(this);

			std::string key = obj->getObjectID()->toString();

			if (AbstractRegistry::lockExists(key))
			{
				if (!AbstractRegistry::isSelfOwned(key))
					return true;
			}
			return false;
		}

		
		bool JDObjectLocker::getLockedObjects(std::vector<LockData>& lockedObjectsOut, Error& err) const
		{
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_5);
			lockedObjectsOut.clear();

			if (!AbstractRegistry::openRegistryFile(m_registryOpenTimeoutMs))
				return false;
			AbstractRegistry::AutoClose autoClose(this);

			std::vector<std::shared_ptr<LockEntryObjectImpl>> loadedObjects;
			if (!AbstractRegistry::readObjects<LockEntryObjectImpl>(loadedObjects))
			{
				err = Error::corruptRegistryFileData;
				return false;
			}
			bool success = true;
			lockedObjectsOut.reserve(loadedObjects.size());
			for(auto obj : loadedObjects)
			{
				if(obj->data.objectID != JDObjectID::invalidID)
					lockedObjectsOut.push_back(obj->data);
				else
				{
					if(m_logger)m_logger->logWarning("Object has empty objectID: "
						+ obj->toString() + "\n");
					err = corruptRegistryFileData;
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
				//m_manager.getSignals().lockedObjectsChanged.emitSignal();
				emit m_manager.lockedObjectsChanged();
			}
		}

		bool JDObjectLocker::lockObjects_internal(const std::vector<JDObject>& objs, std::vector<Error>& errors)
		{
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_10);
			errors.resize(objs.size());
			if (!AbstractRegistry::openRegistryFile(m_registryOpenTimeoutMs))
			{
				for (size_t i = 0; i < objs.size(); ++i)
				{
					errors[i] = Error::cantOpenRegistryFile;
				}
				return false;
			}
			AbstractRegistry::AutoClose autoClose(this);
			AbstractRegistry::removeInactiveObjects();

			std::vector<std::shared_ptr<LockEntryObjectImpl>> loadedObjects;
			if (!AbstractRegistry::readObjects<LockEntryObjectImpl>(loadedObjects))
			{
				for (size_t i = 0; i < objs.size(); ++i)
				{
					errors[i] = Error::corruptRegistryFileData;
				}
				return false;
			}

			// Check if Lock is already aquired
			Utilities::JDUser user = m_manager.getUser();
			std::unordered_map<JDObjectID::IDType, bool> alreadyLocked;
			std::unordered_map<JDObjectID::IDType, bool> lockedByOther;
			for (size_t i = 0; i < loadedObjects.size(); ++i)
			{
				if (loadedObjects[i]->data.user.getSessionID() == user.getSessionID())
				{
					alreadyLocked[loadedObjects[i]->data.objectID] = true;
				}
				else
				{
					lockedByOther[loadedObjects[i]->data.objectID] = true;
				}
			}

			std::vector<std::shared_ptr<LockEntryObject>> newEntries;
			std::unordered_map<JDObjectID::IDType, std::shared_ptr<LockEntryObjectImpl>> alreadyLockedObjects;
			for (size_t i = 0; i < objs.size(); ++i)
			{
				if (!objs[i])
				{
					errors[i] = Error::objIsNullptr;
					continue;
				}
				errors[i] = Error::none;
				const JDObject& obj = objs[i];
				JDObjectID::IDType id = obj->getObjectID()->get();
				std::string key = obj->getObjectID()->toString();
				if (alreadyLocked.find(id) != alreadyLocked.end())
					continue;
				if (lockedByOther.find(id) != lockedByOther.end())
				{
					std::shared_ptr<LockEntryObjectImpl> lockEntry;
					for (auto entry : loadedObjects)
					{
						if (entry->data.objectID == id)
						{
							lockEntry = entry;
							alreadyLockedObjects[id] = entry;
							break;
						}
					}
					errors[i] = Error::objectLockedByOther;
					continue;
				}
				auto newEntry = std::make_shared<LockEntryObjectImpl>(key, obj, m_manager);
				newEntries.push_back(newEntry);
			}

			int ret = 0;
			if ((ret = AbstractRegistry::addObjects(newEntries)) != newEntries.size())
			{
				for (size_t i = 0; i < errors.size(); ++i)
				{
					if (errors[i] == Error::none)
						errors[i] = Error::unableToLockObject;
				}
			}
			if (m_logger)
			{
				for (size_t i = 0; i < objs.size(); ++i)
				{
					switch (errors[i])
					{
						case Error::none:
						{
							m_logger->logInfo("Object: \"" + objs[i]->getObjectID()->toString() + "\" locked");
							break;
						}
						case Error::objectLockedByOther:
						{
							auto alreadyLockedObj = alreadyLockedObjects.find(objs[i]->getObjectID()->get());
							if (alreadyLockedObj != alreadyLockedObjects.end())
							{
								m_logger->logError("Can't lock Object: \"" + objs[i]->getObjectID()->toString() + "\",\nalready locked by user:" +
												   alreadyLockedObj->second->data.user.toString() +
												   "\nLock data:\n" + alreadyLockedObj->second->toString());
							}
							else
								m_logger->logError("Can't lock object: \"" + objs[i]->getObjectID()->toString() + "\"");
							break;
						}
						case Error::objIsNullptr:
						{
							m_logger->logError("Object is nullptr");
							break;
						}
					}
				}
			}
			return newEntries.size() == objs.size() && newEntries.size() == ret;
		}
		bool JDObjectLocker::unlockObjects_internal(const std::vector<JDObject>& objs, std::vector<Error>& errors)
		{
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_10);
			errors.resize(objs.size());
			if (!AbstractRegistry::openRegistryFile(m_registryOpenTimeoutMs))
			{
				for (size_t i = 0; i < objs.size(); ++i)
				{
					errors[i] = Error::cantOpenRegistryFile;
				}
				return false;
			}
			AbstractRegistry::AutoClose autoClose(this);
			std::vector<std::shared_ptr<LockEntryObjectImpl>> loadedObjects;
			std::unordered_map<JDObjectID::IDType, std::shared_ptr<LockEntryObjectImpl>> lockedObjects;
			if (!AbstractRegistry::readObjects<LockEntryObjectImpl>(loadedObjects))
			{
				for (size_t i = 0; i < objs.size(); ++i)
				{
					errors[i] = Error::corruptRegistryFileData;
				}
				return false;
			}
			for (auto obj : loadedObjects)
			{
				lockedObjects[obj->data.objectID] = obj;
			}


			std::vector<std::string> removingKeys;
			removingKeys.reserve(objs.size());

			for (size_t i = 0; i < objs.size(); ++i)
			{
				if (!objs[i])
				{
					errors[i] = Error::objIsNullptr;
					continue;
				}
				errors[i] = Error::none;
				const JDObject& obj = objs[i];
				std::string key = obj->getObjectID()->toString();

				auto lockEntry = lockedObjects.find(obj->getObjectID()->get());
				if (lockEntry == lockedObjects.end())
				{
					errors[i] = Error::objectNotLocked;
					continue;
				}
				else
				{
					if (lockEntry->second->data.user.getSessionID() != m_manager.getUser().getSessionID())
					{
						errors[i] = Error::objectLockedByOther;
						continue;
					}
				}
				removingKeys.push_back(key);
			}

			int removed = 0;
			if ((removed = AbstractRegistry::removeObjects(removingKeys)) != removingKeys.size())
			{
				for (size_t i = 0; i < removingKeys.size(); ++i)
				{
					if (errors[i] == Error::none)
						errors[i] = Error::unableToUnlockObject;
				}
			}

			if (m_logger)
			{
				for (size_t i = 0; i < objs.size(); ++i)
				{
					switch (errors[i])
					{
						case Error::none:
						{
							m_logger->logInfo("Object: \"" + objs[i]->getObjectID()->toString() + "\" unlocked");
							break;
						}
						case Error::objectNotLocked:
						{
							m_logger->logWarning("Lock for object: \"" + objs[i]->getObjectID()->toString() + "\" did not exist");
							break;
						}
						case Error::objectLockedByOther:
						{
							auto alreadyLockedObj = lockedObjects.find(objs[i]->getObjectID()->get())->second;
							m_logger->logError("Can't unlock Object: \"" + objs[i]->getObjectID()->toString() + "\",\nObject locked by user:" +
												 alreadyLockedObj->data.user.toString() +
												 "\nLock data:\n" + alreadyLockedObj->toString());
							break;
						}
						case Error::unableToUnlockObject:
						{
							m_logger->logError("Can't unlock object: \"" + objs[i]->getObjectID()->toString() + "\"");
							break;
						}
						case Error::objIsNullptr:
						{
							m_logger->logError("Object is nullptr");
							break;
						}
					}
				}
			}

			return removingKeys.size() == objs.size() && removingKeys.size() == removed;
		}

		void JDObjectLocker::onCreateFiles()
		{
			m_lockTableWatcher.setup(AbstractRegistry::getRegistrationFilePath(), m_logger);
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
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_11);
			bool success = LockEntryObject::load(obj);
			if(!isValid(obj))
				return false;
			data.objectID = obj.at(JsonKeys::objectID).get<JDObjectID::IDType>();
			data.user.load(obj.at(JsonKeys::user).get<JsonObject>());
			
			data.lockDate = Utilities::fastStringToQDate(obj.at(JsonKeys::lockDate).get<std::string>());
			//data.lockDate = Utilities::stringToQDate(obj.at(JsonKeys::lockDate).get<std::string>());
			data.lockTime = Utilities::fastStringToQTime(obj.at(JsonKeys::lockTime).get<std::string>());
			//data.lockTime = Utilities::stringToQTime(obj.at(JsonKeys::lockTime).get<std::string>());

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
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_12);
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