#include "manager/JDManagerObjectManager.h"
#include "manager/JDManager.h"
#include "object/JDObjectInterface.h"
#include "manager/async/WorkProgress.h"
#include "utilities/JsonUtilities.h"

namespace JsonDatabase
{
    namespace Internal
    {
        JDManagerObjectManager::JDManagerObjectManager(JDManager& manager, std::mutex& mtx)
            : m_manager(manager)
            , m_mutex(mtx)
            , m_objLocker(manager)
        {   }
        JDManagerObjectManager::~JDManagerObjectManager()
        {
           // stop();
            delete m_logger;
        }
        void JDManagerObjectManager::setParentLogger(Log::LogObject* parentLogger)
        {
            if (parentLogger)
            {
                if (m_logger)
                    delete m_logger;
                m_logger = new Log::LogObject(*parentLogger,"JDManagerObjectManager");
                m_objLocker.setParentLogger(m_logger, "Object locker");
            }
        }
        bool JDManagerObjectManager::setup()
        {
            m_objLocker.setDatabasePath(m_manager.getDatabasePath());
            m_objLocker.createFiles();

            m_objLocker.removeInactiveObjectLocks();
            return true;
        }
        bool JDManagerObjectManager::stop()
        {
            Error err;
            m_objLocker.unlockAllObjs(err);
            return true;
        }
        

        void JDManagerObjectManager::setDomainName(const std::string& name)
        {
			m_idDomain.setName(name);
        }

        
        
        bool JDManagerObjectManager::addObject(JDObject obj)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
			bool success = false;
			{
				JDM_UNIQUE_LOCK_P_M(m_objsMutex);
				success = packAndAddObject_internal(obj);
			}
            if (success)
            {
                Error lockErr;
				m_manager.lockObject(obj, lockErr);
                if (lockErr == Error::none)
                {
                    m_manager.saveObjects_internal({ obj }, 1000, nullptr, false);
                    m_manager.unlockObject(obj, lockErr);
                }
            }
            return success;
        }
        bool JDManagerObjectManager::addObject(const std::vector<JDObject>& objList)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
            std::vector<JDObject> addedObjs;
            addedObjs.reserve(objList.size());
            bool success = true;
            {
                JDM_UNIQUE_LOCK_P_M(m_objsMutex);
                for (size_t i = 0; i < objList.size(); ++i)
                {
                    const JDObject &obj = objList[i];
                    bool added = packAndAddObject_internal(obj);
					success &= added;
					if (added)
					{
						addedObjs.push_back(obj);
					}
                }
            }

            if (addedObjs.size() > 0)
            {
                std::vector<Error> lockErr;
                m_manager.lockObjects(addedObjs, lockErr);
                m_manager.saveObjects_internal(addedObjs, 1000, nullptr, false);
                m_manager.unlockObjects(addedObjs, lockErr);
            }
            return success;
        }
        
        bool JDManagerObjectManager::removeObject(JDObject obj)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
			JDM_UNIQUE_LOCK_P_M(m_objsMutex);
            return removeObject_internal(obj);
        }
        
        bool JDManagerObjectManager::removeObjects(const std::vector<JDObject>& objList)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
            bool success = true;
			JDM_UNIQUE_LOCK_P_M(m_objsMutex);
			for (size_t i = 0; i < objList.size(); ++i)
			{
                success &= removeObject_internal(objList[i]);
			}
            return success;
        }
        std::size_t JDManagerObjectManager::getObjectCount() const
        {
            JDM_UNIQUE_LOCK_P_M(m_objsMutex);
            return m_objs.size();
        }
        bool JDManagerObjectManager::exists(JDObject obj) const
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
            JDM_UNIQUE_LOCK_P_M(m_objsMutex);
            return exists_internal(obj);
        }
        bool JDManagerObjectManager::exists(const std::vector<JDObject>& objs) const
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
            JDM_UNIQUE_LOCK_P_M(m_objsMutex);
            return exists_internal(objs);
        }
        bool JDManagerObjectManager::exists(const JDObjectIDptr &id) const
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
            JDM_UNIQUE_LOCK_P_M(m_objsMutex);
            return exists_internal(id);
        }
        JDObject JDManagerObjectManager::getObject(const JDObjectIDptr &id)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
            JDM_UNIQUE_LOCK_P_M(m_objsMutex);
            return getObject_internal(id);
        }
        JDObject JDManagerObjectManager::getObject(const JDObjectID::IDType& id)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
            JDM_UNIQUE_LOCK_P_M(m_objsMutex);
            return getObject_internal(id);
        }
        std::vector<JDObject> JDManagerObjectManager::getObjects() const
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
            JDM_UNIQUE_LOCK_P_M(m_objsMutex);
            return getObjects_internal();
        }

        void JDManagerObjectManager::clearObjects()
        {
            JDM_UNIQUE_LOCK_P_M(m_objsMutex);
            clearObjects_internal();
        }



        bool JDManagerObjectManager::lockObject(const JDObject& obj, Error& err)
        {
            JDM_UNIQUE_LOCK_P_M(m_objsMutex);
			bool ret = m_objLocker.lockObject(obj, err);
            if (ret)
                m_manager.m_signalsToEmit.addObjectLocked(obj);
            return ret;
        }
        bool JDManagerObjectManager::lockObjects(const std::vector<JDObject>& objs, std::vector<Error>& errors)
        {
			JDM_UNIQUE_LOCK_P_M(m_objsMutex);
			bool ret = m_objLocker.lockObjects(objs, errors);
            for (size_t i = 0; i < errors.size(); ++i)
            {
                if (errors[i] == Error::none)
                    m_manager.m_signalsToEmit.addObjectLocked(objs[i]);
            }
			return ret;
        }
        bool JDManagerObjectManager::unlockObject(const JDObject& obj, Error& err)
        {
            JDM_UNIQUE_LOCK_P_M(m_objsMutex);
			bool ret = m_objLocker.unlockObject(obj, err);
            if(ret)
				m_manager.m_signalsToEmit.addObjectUnlocked(obj);
            return ret;
        }
        bool JDManagerObjectManager::unlockObject(const JDObjectID::IDType& id, Error& err)
        {
            JDM_UNIQUE_LOCK_P_M(m_objsMutex);
            JDObject obj = m_manager.getObject_internal(id);
            bool ret = m_objLocker.unlockObject(obj, err);
            if (ret)
                m_manager.m_signalsToEmit.addObjectUnlocked(obj);
            return ret;
		}
        bool JDManagerObjectManager::unlockObjects(const std::vector<JDObject>& objs, std::vector<Error>& errors)
        {
			JDM_UNIQUE_LOCK_P_M(m_objsMutex);
            bool ret = m_objLocker.unlockObjects(objs, errors);
            for (size_t i = 0; i < errors.size(); ++i)
            {
                if (errors[i] == Error::none)
                    m_manager.m_signalsToEmit.addObjectUnlocked(objs[i]);
            }
            return ret;
        }
        bool JDManagerObjectManager::lockAllObjs(Error& err)
        {
            std::vector<Error> errs;
            err = Error::none;
			bool ret = lockObjects(getObjects_internal(), errs);
            if (!ret)
                err = Error::unableToLockObject;
            return ret;
		}
        bool JDManagerObjectManager::unlockAllObjs(Error& err)
        {
            std::vector<Error> errs;
            err = Error::none;
			bool ret = unlockObjects(getObjects_internal(), errs);
            if(!ret)
				err = Error::unableToUnlockObject;
            return ret;
        }
        bool JDManagerObjectManager::isObjectLocked(const JDObject& obj, Error& err) const
        {
            JDM_UNIQUE_LOCK_P_M(m_objsMutex);
            return m_objLocker.isObjectLocked(obj, err);
        }
        bool JDManagerObjectManager::isObjectLockedByMe(const JDObject& obj, Error& err) const
        {
            JDM_UNIQUE_LOCK_P_M(m_objsMutex);
            return m_objLocker.isObjectLockedByMe(obj, err);
        }
        bool JDManagerObjectManager::isObjectLockedByOther(const JDObject& obj, Error& err) const
        {
            JDM_UNIQUE_LOCK_P_M(m_objsMutex);
            return m_objLocker.isObjectLockedByOther(obj, err);
        }
        bool JDManagerObjectManager::getObjectLocks(std::vector<JDObjectLocker::LockData>& lockedObjectsOut, Error& err) const
        {
            JDM_UNIQUE_LOCK_P_M(m_objsMutex);
            return m_objLocker.getLockedObjects(lockedObjectsOut, err);
        }
        bool JDManagerObjectManager::getObjectLocksByUser(
            const Utilities::JDUser& user, 
            std::vector<JDObjectLocker::LockData>& lockedObjectsOut, 
            Error& err) const
        {
            JDM_UNIQUE_LOCK_P_M(m_objsMutex);
			return getObjectLocksByUser_internal(user, lockedObjectsOut, err);
        }
   
        bool JDManagerObjectManager::getLockedObjects(std::vector<LockedObject>& lockedObjectsOut, Error& err) const
        {
            JDM_UNIQUE_LOCK_P_M(m_objsMutex);
			return getLockedObjects_internal(lockedObjectsOut, err);
        }
        bool JDManagerObjectManager::getLockedObjects(std::vector<JDObject>& lockedObjectsOut, Error& err) const
        {
            JDM_UNIQUE_LOCK_P_M(m_objsMutex);
            return getLockedObjects_internal(lockedObjectsOut, err);
        }
        bool JDManagerObjectManager::getLockedObjects(const Utilities::JDUser& user, std::vector<JDObject>& lockedObjectsOut, Error& err) const
        {
            JDM_UNIQUE_LOCK_P_M(m_objsMutex);
			return getLockedObjects_internal(user, lockedObjectsOut, err);
        }
        bool JDManagerObjectManager::getLockOwner(const JDObject& obj, Utilities::JDUser& userOut, Error& err) const
        {
            JDM_UNIQUE_LOCK_P_M(m_objsMutex);
			return getLockOwner_internal(obj, userOut, err);
        }
        bool JDManagerObjectManager::getLockData(const JDObject& obj, JDObjectLocker::LockData& lockDataOut, Error& err) const
        {
            JDM_UNIQUE_LOCK_P_M(m_objsMutex);
			return getLockData_internal(obj, lockDataOut, err);
        }
        int JDManagerObjectManager::removeInactiveObjectLocks() const
        {
            JDM_UNIQUE_LOCK_P_M(m_objsMutex);
            return m_objLocker.removeInactiveObjectLocks();
        }

        /*
          -----------------------------------------------------------------------------------------------
          ------------------ I N T E R N A L ------------------------------------------------------------
          -----------------------------------------------------------------------------------------------
        */
        void JDManagerObjectManager::onDatabasePathChange(const std::string& oldPath, const std::string& newPath)
        {
            JD_UNUSED(oldPath);
            m_objLocker.setDatabasePath(newPath+ "\\"+m_manager.getDatabaseName());
        }

        bool JDManagerObjectManager::objectIDIsValid(const JDObjectIDptr& id) const
        {
            if (!id.get())
				return false;
            else
            {
                return id->isValid();
			}
			return true;
        }
        bool JDManagerObjectManager::objectIDIsValid(const JDObject& obj) const
        {
            JDObjectIDptr id = obj->getObjectID();
            return objectIDIsValid(id);
        }

       /* void JDManagerObjectManager::checkObjectIDAndFix_internal(const JDObject& obj)
        {
            if(!objectIDIsValid(obj))
                newObjectInstantiated_internal(obj);
        }

        void JDManagerObjectManager::newObjectInstantiated_internal(const JDObject& obj)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            JDObjectIDptr id = m_idDomain.getNewID();
            obj->setObjectID(id);
        }*/
        bool JDManagerObjectManager::packAndAddObject_internal(const JDObject& obj)
        { 
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            if(m_objs.exists(obj) || !obj.get())
				return false; // Object already added
            if (obj->isManaged())
                return false; // Object already managed
			
            JDObjectIDptr id = m_idDomain.getNewID();
           /* for (size_t i = 0; i< m_removedObjectIDs.size(); ++i)
            {
                if (m_removedObjectIDs[i] == id->get())
                {
					m_removedObjectIDs.erase(m_removedObjectIDs.begin() + i);
					break;
				}
			}*/
            JDObjectManager *manager = new JDObjectManager(&m_manager, obj, id, m_logger);
            if(m_objs.addObject(manager))
            {
				m_manager.m_signalsToEmit.addObjectAdded(obj);
                if(m_logger)
					m_logger->logInfo("Added object with ID: " + obj->getObjectID().get()->toString());
				
				return true;
			}
            if(m_logger)
                m_logger->logWarning("Failed to add object with ID: " + obj->getObjectID().get()->toString());
            return false;
			//return addObject_internal(manager);
        }
        bool JDManagerObjectManager::packAndAddObject_internal(const JDObject& obj, const JDObjectID::IDType& presetID)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            if (m_objs.exists(obj) || !obj.get())
                return false; // Object already added
            if (obj->isManaged())
                return false; // Object already managed
            bool success;
            JDObjectIDptr id = m_idDomain.getPredefinedID(presetID, success);
            if(!success)
            {
#ifdef JD_DEBUG
                if (m_idDomain.getExistingID(presetID))
                {
                    if(m_logger)m_logger->logError("Failed to add object with preset ID: " + std::to_string(presetID) + " ID already exists");
                }
                else
                {
                    if(m_logger)m_logger->logError("Failed to add object with preset ID: " + std::to_string(presetID) + " ID unknown fail");
                }
#endif
                return false; 
            }
           /* for (size_t i = 0; i < m_removedObjectIDs.size(); ++i)
            {
                if (m_removedObjectIDs[i] == id->get())
                {
                    m_removedObjectIDs.erase(m_removedObjectIDs.begin() + i);
                    break;
                }
            }*/
            JDObjectManager* manager = new JDObjectManager(&m_manager, obj, id, m_logger);
            success &= m_objs.addObject(manager);
            if(success)
                m_manager.m_signalsToEmit.addObjectAdded(obj);
            return success;
        }
        bool JDManagerObjectManager::packAndAddObject_internal(const std::vector<JDObject>& objs)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            m_objs.reserve(m_objs.size() + objs.size());
            bool success = true;
            for (size_t i = 0; i < objs.size(); ++i)
            {
                success &= packAndAddObject_internal(objs[i]);
            }
			return success;
        }
        bool JDManagerObjectManager::packAndAddObject_internal(const std::vector<JDObjectID::IDType>& ids, const std::vector<JDObject>& objs)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            m_objs.reserve(m_objs.size() + objs.size());
            bool success = true;
            std::vector<JDObjectIDptr> generatedIDs = m_idDomain.getPredefinedIDs(ids, success);

            m_objs.reserve(m_objs.size() + objs.size());
            for (size_t i = 0; i < objs.size(); ++i)
            {
               /* for (size_t j = 0; j < m_removedObjectIDs.size(); ++j)
                {
                    if (m_removedObjectIDs[j] == generatedIDs[i]->get())
                    {
                        m_removedObjectIDs.erase(m_removedObjectIDs.begin() + j);
                        break;
                    }
                }*/
                JDObjectManager* manager = new JDObjectManager(&m_manager, objs[i], generatedIDs[i], m_logger);
                bool success2 = m_objs.addObject(manager);
                if(success2)
                    m_manager.m_signalsToEmit.addObjectAdded(objs[i]);
                success &= success2;
                if (m_logger)
                {
                    if(success2)
                    {
                        m_logger->logInfo("Added object with ID: " + objs[i]->getObjectID().get()->toString());
					}
                    else
                    {
                        m_logger->logWarning("Failed to add object with ID: " + objs[i]->getObjectID().get()->toString());
                    }
                }
            }
            
           // for (size_t i = 0; i < objs.size(); ++i)
           //     success &= packAndAddObject_internal(ids[i], objs[i]);
            return success;
        }

        /*JDObject JDManagerObjectManager::replaceObject_internal(const JDObject& obj)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);

            
            JDObjectID::IDType id = obj->getShallowObjectID();
            JDObjectManager* replacedManager = m_objs.getObjectByID(id);
            if(!replacedManager)
                return nullptr; // Object not found



            JDObject replacedObj = replacedManager->getObject();
            if(!m_objs.removeObject(replacedObj->getObjectID()))
				return nullptr; // Object not found

            JDObjectIDptr idPtr = replacedObj->getObjectID();
            delete replacedManager;
            replacedManager = nullptr;
            
            JDObjectManager* newManager = new JDObjectManager(&m_manager, obj, idPtr, m_logger);
            m_objs.addObject(newManager);
            if(m_logger)
                m_logger->logInfo("Replaced object with ID: " + obj->getObjectID().get()->toString());
            return replacedObj;
        }
        void JDManagerObjectManager::replaceObject_internal(const std::vector<JDObject>& objs)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            for (size_t i = 0; i < objs.size(); ++i)
            {
                replaceObject_internal(objs[i]);
            }
        }*/
        bool JDManagerObjectManager::removeObject_internal(const JDObject & obj, bool doSave)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            if (!obj.get())
            {
                if(m_logger)
                    m_logger->logError("Can't remove object. Object is nullptr");
                return false;
            }

            // Check if Object is locked by this user
            if (doSave)
            {
                Error err;
                if (!m_objLocker.isObjectLockedByMe(obj, err))
                {
                    if (m_logger)
                        m_logger->logWarning("Can't remove object with ID: " + obj->getObjectID().get()->toString() + ". Object is not locked by this user.");
                    return false;
                }
            }

            JDObjectManager* removedManager = m_objs.getAndRemoveObject(obj->getObjectID());
            bool removed = removedManager != nullptr;
            if (removed)
            {
                //m_removedObjectIDs.push_back(obj->getObjectID().get()->get());
               
                if(doSave)
                    m_manager.saveObjects_internal({ obj }, 1000, nullptr, true);
                Error err;
                m_objLocker.unlockObject(obj, err);
                m_idDomain.unregisterID(obj->getObjectID());
				delete removedManager;
				m_manager.m_signalsToEmit.addObjectRemoved(obj);
                
                
			}
            if(m_logger)
                m_logger->logInfo("Removed object with ID: " + JDObjectID::idToStr(obj->getShallowObjectID()));
            return removed;
        }
        bool JDManagerObjectManager::removeObject_internal(const std::vector<JDObject>& objs, bool doSave)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            if(m_objs.removeObject(objs))
            {				
                if (doSave)
                {
                    Error err;
                    std::vector<JDObjectLocker::LockData> lockedObjects;
                    if (!m_objLocker.getLockedObjects(lockedObjects, err))
                    {
						if (m_logger)
							m_logger->logError("Can't remove objects. Can't get locked objects.");
                        return false;
                    }

                    std::vector<JDObject> lockedObjs;
                    for (size_t i = 0; i < objs.size(); ++i)
                    {
                        bool isLocked = false;
                        // Check if Object is locked by this user
                        for (size_t j = 0; j < lockedObjects.size(); ++j)
                        {
							if (lockedObjects[j].objectID == objs[i]->getObjectID()->get())
							{
								lockedObjs.push_back(objs[i]);
                                isLocked = true;
								break;
							}
                        }
                        
                        if (!isLocked)
                        {
                            if (m_logger)
                                m_logger->logWarning("Can't remove object with ID: " + objs[i]->getObjectID().get()->toString() + ". Object is not locked by this user.");
                        }
                    }
                    std::vector<Error> errors(lockedObjs.size());
                    m_manager.saveObjects_internal(lockedObjs, 1000, nullptr, true);
                    m_objLocker.unlockObjects(lockedObjs, errors);
                }


                
                for(size_t i=0; i< objs.size(); ++i)
				{
                    //m_removedObjectIDs.push_back(objs[i]->getObjectID().get()->get());
                    m_idDomain.unregisterID(objs[i]->getObjectID());
					JDObjectManager* manager = objs[i]->getManager();
					delete manager;
                    m_manager.m_signalsToEmit.addObjectRemoved(objs[i]);
                    if (m_logger)
                        m_logger->logInfo("Removed object with ID: " + JDObjectID::idToStr(objs[i]->getShallowObjectID()));
				}
               
				
                return true;
			}  
            return false;
        }
        bool JDManagerObjectManager::exists_internal(const JDObject & obj) const
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            return m_objs.exists(obj);
        }
        bool JDManagerObjectManager::exists_internal(const std::vector<JDObject>& objs) const
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            for (size_t i = 0; i < objs.size(); ++i)
            {
                if (!m_objs.exists(objs[i]))
                    return false;
            }
            return true;
        }
        bool JDManagerObjectManager::exists_internal(const JDObjectIDptr &id) const
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            return m_objs.exists(id);
        }
        JDObject JDManagerObjectManager::getObject_internal(const JDObjectIDptr& id) const
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            JDObjectManager *manager = m_objs.getObjectByID(id);
            if(manager)
                return manager->getObject();
            return nullptr;
        }
        JDObject JDManagerObjectManager::getObject_internal(const JDObjectID::IDType& id) const
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            JDObjectManager* manager = m_objs.getObjectByID(id);
            if (manager)
                return manager->getObject();
            return nullptr;
        }
        std::vector<JDObject> JDManagerObjectManager::getObjects_internal() const
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            auto managers = m_objs.getAllObjects();
            std::vector<JDObject> objs(managers.size());
            for(size_t i=0; i< managers.size(); ++i)
                objs[i] = managers[i]->getObject();
            
            return objs;
        }
        JDObjectManager* JDManagerObjectManager::getObjectManager_internal(const JDObjectIDptr& id)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            return m_objs.getObjectByID(id);
        }
        JDObjectManager* JDManagerObjectManager::getObjectManager_internal(const JDObjectID::IDType& id)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            return m_objs.getObjectByID(id);
        }
        const std::vector<JDObjectManager*>& JDManagerObjectManager::getObjectManagers_internal() const
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            return m_objs.getAllObjects();
        }
        void JDManagerObjectManager::clearObjects_internal()
        {
            auto objs = getObjects_internal();
            removeObject_internal(objs, true);
        }

        bool JDManagerObjectManager::getObjectLocksByUser_internal(const Utilities::JDUser& user, std::vector<JDObjectLocker::LockData>& lockedObjectsOut, Error& err) const
        {
            std::vector<JDObjectLocker::LockData> tmp;
            if (!m_objLocker.getLockedObjects(tmp, err))
                return false;
            lockedObjectsOut.clear();
            for (size_t i = 0; i < tmp.size(); ++i)
            {
                if (tmp[i].user.getSessionID() == user.getSessionID())
                    lockedObjectsOut.push_back(tmp[i]);
            }
            return true;
        }


        bool JDManagerObjectManager::getLockedObjects_internal(std::vector<LockedObject>& lockedObjectsOut, Error& err) const
        {
            std::vector<JDObjectLocker::LockData> tmp;
            if (!m_objLocker.getLockedObjects(tmp, err))
                return false;
            lockedObjectsOut.clear();
            for (size_t i = 0; i < tmp.size(); ++i)
            {
                LockedObject data;
                data.obj = getObject_internal(tmp[i].objectID);
                data.lockData = tmp[i];
                lockedObjectsOut.push_back(data);
            }
            return true;
        }
        bool JDManagerObjectManager::getLockedObjects_internal(std::vector<JDObject>& lockedObjectsOut, Error& err) const
        {
            std::vector<JDObjectLocker::LockData> tmp;
            if (!m_objLocker.getLockedObjects(tmp, err))
                return false;
            lockedObjectsOut.clear();
            for (size_t i = 0; i < tmp.size(); ++i)
            {
                lockedObjectsOut.push_back(getObject_internal(tmp[i].objectID));
            }
            return true;
        }
        bool JDManagerObjectManager::getLockedObjects_internal(const Utilities::JDUser& user, std::vector<JDObject>& lockedObjectsOut, Error& err) const
        {
            std::vector<JDObjectLocker::LockData> locks;
            lockedObjectsOut.clear();
            err = Error::none;
            if (getObjectLocksByUser_internal(user, locks, err))
            {
                lockedObjectsOut.reserve(locks.size());
                for (auto lock : locks)
                {
                    lockedObjectsOut.push_back(m_manager.getObject_internal(lock.objectID));
                }
                return true;
            }
            return false;
        }
        bool JDManagerObjectManager::getLockOwner_internal(const JDObject& obj, Utilities::JDUser& userOut, Error& err) const
        {
            if (!obj)
            {
                err = Error::objIsNullptr;
                return false;
            }
            std::vector<JDObjectLocker::LockData> lockedObjects;
            if (!m_objLocker.getLockedObjects(lockedObjects, err))
                return false;
            for (size_t i = 0; i < lockedObjects.size(); ++i)
            {
                JDObjectID::IDType id = obj->getShallowObjectID();
                if (obj->getObjectID())
                    id = obj->getObjectID()->get();
                if (lockedObjects[i].objectID == id)
                {
                    userOut = lockedObjects[i].user;
                    return true;
                }
            }
            err = Error::objectNotLocked;
            return false;
        }
        bool JDManagerObjectManager::getLockData_internal(const JDObject& obj, JDObjectLocker::LockData& lockDataOut, Error& err) const
        {
            if (!obj)
            {
                err = Error::objIsNullptr;
                return false;
            }
            err = Error::none;
            return m_objLocker.getLockData(obj->getShallowObjectID(), lockDataOut, err);
        }
        bool JDManagerObjectManager::loadObjectFromJson_internal(const JsonObject& json, const JDObject& obj)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            if (!obj->loadInternal(json))
            {
                if (m_logger)m_logger->logError("Can't load data in object: " + obj->getObjectID().get()->toString() + " classType: " + obj->className());
                return false;
            }
            return true;
        }



        bool JDManagerObjectManager::loadObjectsFromJson_internal(const JsonArray& jsons, int mode, Internal::WorkProgress* progress,
            std::vector<JDObject> &overridingObjs,
            std::vector<JDObjectID::IDType>& newObjIDs,
            std::vector<JDObject>& newObjInstances,
            std::vector<JDObject> &removedObjs,
            std::vector<JDObjectPair> &changedPairs)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            double progressScalar = 0;
            if (progress)
            {
                progressScalar = progress->getScalar();
            }
            bool success = true;

            bool modeNewObjects = (mode & (int)LoadMode::newObjects);
            bool modeChangedObjects = (mode & (int)LoadMode::changedObjects);
            bool modeRemovedObjects = (mode & (int)LoadMode::removedObjects);

           // bool overrideChanges = (mode & (int)LoadMode::overrideChanges);

            std::vector<JDObject> replaceObjs;
            std::unordered_map<JDObject, JDObject> loadedObjects;

            // Prepare the data for the loader
            JDObjectManager::ManagedLoadContainers loaderContainers {
                .overridingObjs = overridingObjs,
                .newObjIDs = newObjIDs,
                .newObjInstances = newObjInstances,
                .changedPairs = changedPairs,
                .replaceObjs = replaceObjs,
                .loadedObjects = loadedObjects 
            };
            JDObjectManager::ManagedLoadMode loadMode {
				.newObjects = modeNewObjects,
                .removedObjects = modeRemovedObjects,
				.changedObjects = modeChangedObjects,
			//	.overridingObjects = overrideChanges
			};

            if (progress)
            {
                progress->setComment("Loading " + std::to_string(jsons.size()) + " objects");
                progress->startNewSubProgress(progressScalar * 0.6);
            }
            
            JD_GENERAL_PROFILING_BLOCK("Load objects", JD_COLOR_STAGE_3);
            size_t jsonCount = jsons.size();
            overridingObjs.reserve(jsonCount);
            newObjIDs.reserve(jsonCount);
            newObjInstances.reserve(jsonCount);
            changedPairs.reserve(jsonCount);
            replaceObjs.reserve(jsonCount);
            //replaceObjs.reserve(jsonCount);
            loadedObjects.reserve(jsonCount);

            double factor = 1 / (double)jsonCount;
            for (size_t i = 0; i < jsons.size(); ++i)
            {
                JDObjectManager::ManagedLoadMisc loaderMisc;
                bool loaded = false;
              
                if(!jsons[i].holds<JsonObject>())
				{
                    if (m_logger)m_logger->logError("Json data is not an object: \"" + jsons[i].toString() + "\"");
					success = false;
					continue;
				}
                const JsonObject& json = jsons[i].get<JsonObject>();
                
                if (json.contains(JDObjectInterface::s_tag_objID))
                {
                    const JsonValue& idValue = json.at(JDObjectInterface::s_tag_objID);
                    if(idValue.holds<JDObjectID::IDType>())
						loaderMisc.id = idValue.get<JDObjectID::IDType>();
                    else
                    {
#if JD_ID_TYPE_SWITCH == JD_ID_TYPE_STRING
                        if (idValue.holds<long>())
                        {
                            loaderMisc.id = std::to_string(idValue.get<long>());
                        }
                        else
                        {
                            JD_CONSOLE_FUNCTION("Invalid ID type in object: \"" << json << "\"\n");
                            success = false;
                            continue;
                        }
#elif JD_ID_TYPE_SWITCH == JD_ID_TYPE_LONG
                        if (idValue.holds<std::string>())
                        {
                            const std::string &idStr = idValue.get<std::string>();
                            long idValueLong = std::stol(idStr);
                            if(idValueLong < 0)
							{
                                if(m_logger)m_logger->logError("Invalid ID type in object: \"" + JsonValue(json).toString() + "\"");
								success = false;
								continue;
							}
                            if(std::to_string(idValueLong) != idStr)
                            {
                                if (m_logger)m_logger->logError("Invalid ID type in object: \"" + JsonValue(json).toString() + "\"");
								success = false;
								continue;
							}
                            loaderMisc.id = idValueLong;
                        }
                        else
                        {
                            if(m_logger)m_logger->logError("Invalid ID type in object: \"" + JsonValue(json).toString() + "\"");
                            success = false;
                            continue;
                        }
#else
                        if (m_logger)m_logger->logError("Invalid ID type in object: \"" + JsonValue(json).toString() + "\"");
                        success = false;
                        continue;
#endif
					}
                    loaded = true;
                }
				
                if (!loaded)
                {
                    if(m_logger)m_logger->logError("Objet has incomplete data. Key: \"" 
						+ JDObjectInterface::s_tag_objID + "\" is missed\n"
						+ "Object: \"" + JsonValue(json).toString() + "\"");
                    success = false;
                    continue;
                }

                JDObjectManager *manager = getObjectManager_internal(loaderMisc.id);

                JDObjectManager::ManagedLoadStatus status = JDObjectManager::managedLoad(
                    json, manager, loaderContainers, loadMode, loaderMisc, m_logger);

                if(status != JDObjectManager::ManagedLoadStatus::success)
				{
					success = false;
                    if (m_logger)m_logger->logError("Failed to load object with ID: " + std::to_string(loaderMisc.id) + " Error: \""
                        + JDObjectManager::managedLoadStatusToString(status) + "\"");
					continue;
				}
				if (progress)
					progress->setProgress((double)i * factor);
            }
            JD_GENERAL_PROFILING_END_BLOCK;

            JD_GENERAL_PROFILING_BLOCK("Find removed objects", JD_COLOR_STAGE_3);
            // Find new added objects
            if (modeRemovedObjects)
            {
                std::vector<JDObjectManager*> managers = getObjectManagers_internal();

                removedObjs.reserve(managers.size());
                for (auto manager : managers)
                {
                    JDObject obj = manager->getObject();
                    if (loadedObjects.find(obj) != loadedObjects.end())
                        continue;

                    removedObjs.emplace_back(obj);
                nextObj:;
                }
            }

            if (progress)
            {
                progress->setScalar(progressScalar);
                progress->addProgress(0.1);
            }
            JD_GENERAL_PROFILING_END_BLOCK;

           
            int counter = 0;

            JD_GENERAL_PROFILING_BLOCK("Remove objects", JD_COLOR_STAGE_3);
            // Copy the data to the signals
            if (modeRemovedObjects && removedObjs.size())
            {
                if (progress)
                    progress->setComment("Remove " + std::to_string(removedObjs.size()) + " objects");
                success &= removeObject_internal(removedObjs, false);
                if(m_logger)
                    m_logger->logInfo("Removed " + std::to_string(removedObjs.size()) + " objects");
                if (progress)
                {
                    progress->addProgress(0.1);
                    ++counter;
                }
            }
            JD_GENERAL_PROFILING_END_BLOCK;

           /* JD_GENERAL_PROFILING_BLOCK("Replace objects", JD_COLOR_STAGE_3);
            if (modeChangedObjects && !overrideChanges && changedPairs.size())
            {
                if(progress)
                    progress->setComment("Replace " + std::to_string(replaceObjs.size()) + " objects");
                replaceObject_internal(replaceObjs);
                if(m_logger)
                    m_logger->logInfo("Replaced " + std::to_string(replaceObjs.size()) + " objects");
                if (progress)
                {
                    progress->addProgress(0.1);
                    ++counter;
                }
            }
            JD_GENERAL_PROFILING_END_BLOCK;*/

            JD_GENERAL_PROFILING_BLOCK("Add new objects", JD_COLOR_STAGE_3);
            if (modeNewObjects && newObjIDs.size())
            {
                if (progress)
                    progress->setComment("Add " + std::to_string(newObjIDs.size()) + " new objects");
                success &= packAndAddObject_internal(newObjIDs, newObjInstances);
                if (m_logger)
                    m_logger->logInfo("Added " + std::to_string(newObjIDs.size()) + " new objects");
                if (progress)
                {
                    progress->addProgress(0.1);
                    ++counter;
                }
                
            }

            if (overridingObjs.size() && m_logger)
            {
                m_logger->logInfo(std::to_string(newObjIDs.size()) + " overwritten objects");
            }

            if (progress)
            {
                progress->addProgress((3 - counter) * 0.1);
            }
            JD_GENERAL_PROFILING_END_BLOCK;
                
            return success;
        }


        void JDManagerObjectManager::update()
        {
            m_objLocker.update();
        }

    }
}