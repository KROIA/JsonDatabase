#include "manager/JDManagerObjectManager.h"
#include "manager/JDManager.h"
#include "utilities/JDUniqueMutexLock.h"
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
            , m_objLocker(manager, mtx)
        {

        }
        JDManagerObjectManager::~JDManagerObjectManager()
        {

        }
        bool JDManagerObjectManager::setup()
        {
            m_objLocker.setDatabasePath(m_manager.getDatabasePath());
            m_objLocker.createFiles();

            m_objLocker.removeInactiveObjectLocks();
            return true;
        }

        void JDManagerObjectManager::setDomainName(const std::string& name)
        {
			m_idDomain.setName(name);
        }

        
        
        bool JDManagerObjectManager::addObject(JDObject obj)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
            JDM_UNIQUE_LOCK_P_M(m_objsMutex);
            return packAndAddObject_internal(obj);
        }
        bool JDManagerObjectManager::addObject(const std::vector<JDObject>& objList)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
            JDM_UNIQUE_LOCK_P_M(m_objsMutex);
            bool success = true;
            for (size_t i = 0; i < objList.size(); ++i)
            {
                JDObject obj = objList[i];
                success &= packAndAddObject_internal(obj);
            }
            return success;
        }
        
        JDObject JDManagerObjectManager::replaceObject(JDObject obj)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
            JDM_UNIQUE_LOCK_P_M(m_objsMutex);
            if (!objectIDIsValid(obj))
                return nullptr;
            return replaceObject_internal(obj);
        }
        std::vector<JDObject> JDManagerObjectManager::replaceObjects(const std::vector<JDObject>& objList)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
            JDM_UNIQUE_LOCK_P_M(m_objsMutex);
            std::vector<JDObject> replacedObjs;
            replacedObjs.reserve(objList.size());
            for (size_t i = 0; i < objList.size(); ++i)
            {
                replacedObjs.push_back(replaceObject_internal(objList[i]));
            }
            return replacedObjs;
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
            JDM_UNIQUE_LOCK_P_M(m_objsMutex);
            bool success = true;
            for (size_t i = 0; i < objList.size(); ++i)
                success &= removeObject_internal(objList[i]);
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



        bool JDManagerObjectManager::lockObject(const JDObject& obj, JDObjectLocker::Error& err)
        {
            return m_objLocker.lockObject(obj, err);
        }
        bool JDManagerObjectManager::unlockObject(const JDObject& obj, JDObjectLocker::Error& err)
        {
            return m_objLocker.unlockObject(obj, err);
        }
        bool JDManagerObjectManager::unlockAllObjs(JDObjectLocker::Error& err)
        {
            return m_objLocker.unlockAllObjs(err);
        }
        bool JDManagerObjectManager::isObjectLocked(const JDObject& obj, JDObjectLocker::Error& err) const
        {
            return m_objLocker.isObjectLocked(obj, err);
        }
        bool JDManagerObjectManager::isObjectLockedByMe(const JDObject& obj, JDObjectLocker::Error& err) const
        {
            return m_objLocker.isObjectLockedByMe(obj, err);
        }
        bool JDManagerObjectManager::isObjectLockedByOther(const JDObject& obj, JDObjectLocker::Error& err) const
        {
            return m_objLocker.isObjectLockedByOther(obj, err);
        }
        bool JDManagerObjectManager::getLockedObjects(std::vector<JDObjectLocker::LockData>& lockedObjectsOut, JDObjectLocker::Error& err) const
        {
            return m_objLocker.getLockedObjects(lockedObjectsOut, err);
        }
        int JDManagerObjectManager::removeInactiveObjectLocks() const
        {
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
            m_objLocker.setDatabasePath(newPath);
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
            JDObjectManager *manager = new JDObjectManager(obj, id);
            return m_objs.addObject(manager);
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
                    JD_CONSOLE_FUNCTION("Failed to add object with preset ID: " << presetID << " ID already exists\n");
                }
                else
                {
                    JD_CONSOLE_FUNCTION("Failed to add object with preset ID: " << presetID << " ID unknown fail\n");
                }
#endif
                return false; 
            }
            JDObjectManager* manager = new JDObjectManager(obj, id);
            return m_objs.addObject(manager);
        }
        bool JDManagerObjectManager::packAndAddObject_internal(const std::vector<JDObject>& objs)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            m_objs.reserve(m_objs.size() + objs.size());
            bool success = true;
            for (size_t i = 0; i < objs.size(); ++i)
				success &= packAndAddObject_internal(objs[i]);
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
                JDObjectManager* manager = new JDObjectManager(objs[i], generatedIDs[i]);
                success &= m_objs.addObject(manager);
            }
            
           // for (size_t i = 0; i < objs.size(); ++i)
           //     success &= packAndAddObject_internal(ids[i], objs[i]);
            return success;
        }

        JDObject JDManagerObjectManager::replaceObject_internal(const JDObject& obj)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            JDObjectIDptr id = obj->getObjectID();
            if (!JDObjectID::isValid(id))
                return nullptr; // No valid ID

            JDObjectManager* replacedManager = m_objs.getObjectByID(id);
            JDObject replacedObj = replacedManager->getObject();
            if(m_objs.removeObject(id))
				return nullptr; // Object not found

            delete replacedManager;
            replacedManager = nullptr;
            JDObjectManager* newManager = new JDObjectManager(obj, id);
            m_objs.addObject(newManager);
            return replacedObj;
        }
        void JDManagerObjectManager::replaceObject_internal(const std::vector<JDObject>& objs)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            for (size_t i = 0; i < objs.size(); ++i)
            {
                replaceObject_internal(objs[i]);
            }
        }
        bool JDManagerObjectManager::removeObject_internal(const JDObject & obj)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            JDObjectManager* removedManager = m_objs.getAndRemoveObject(obj->getObjectID());
            bool removed = removedManager != nullptr;
            delete removedManager;
            return removed;
        }
        bool JDManagerObjectManager::removeObject_internal(const std::vector<JDObject>& objs)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            return m_objs.removeObject(objs);
        }
        bool JDManagerObjectManager::exists_internal(const JDObject & obj) const
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            return m_objs.exists(obj);
        }
        bool JDManagerObjectManager::exists_internal(const JDObjectIDptr &id) const
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            return m_objs.exists(id);
        }
        JDObject JDManagerObjectManager::getObject_internal(const JDObjectIDptr& id)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            JDObjectManager *manager = m_objs.getObjectByID(id);
            if(manager)
                return manager->getObject();
            return nullptr;
        }
        JDObject JDManagerObjectManager::getObject_internal(const JDObjectID::IDType& id)
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
            m_objs.clear();
        }

#if JD_ACTIVE_JSON == JD_JSON_QT
        bool JDManagerObjectManager::loadObjectFromJson_internal(const QJsonObject& json, const JDObject& obj)
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
        bool JDManagerObjectManager::loadObjectFromJson_internal(const JsonObject& json, const JDObject& obj)
#endif
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            if (!obj->loadInternal(json))
            {
                JD_CONSOLE_FUNCTION("Can't load data in object: " << obj->getObjectID() << " classType: " << obj->className() + "\n");
                return false;
            }
            return true;
        }



#if JD_ACTIVE_JSON == JD_JSON_QT
        bool JDManagerObjectManager::loadObjectsFromJson_internal(const std::vector<QJsonObject>& jsons, int mode, Internal::WorkProgress* progress,
           // bool hasOverrideChangeFromDatabaseSlots,
           // bool hasChangeFromDatabaseSlots,
           // bool hasObjectAddedToDatabaseSlots,
           // bool hasObjectRemovedFromDatabaseSlots,
            std::vector<JDObject>& overridingObjs,
            std::vector<JDObjectID::IDType>& newObjIDs,
            std::vector<JDObject>& newObjInstances,
            std::vector<JDObject>& removedObjs,
            std::vector<JDObjectPair>& changedPairs)
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
        bool JDManagerObjectManager::loadObjectsFromJson_internal(const JsonArray& jsons, int mode, Internal::WorkProgress* progress,
           // bool hasOverrideChangeFromDatabaseSlots,
           // bool hasChangeFromDatabaseSlots,
           // bool hasObjectAddedToDatabaseSlots,
           // bool hasObjectRemovedFromDatabaseSlots,
            std::vector<JDObject> &overridingObjs,
            std::vector<JDObjectID::IDType>& newObjIDs,
            std::vector<JDObject>& newObjInstances,
            std::vector<JDObject> &removedObjs,
            std::vector<JDObjectPair> &changedPairs)
#endif
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

            bool overrideChanges = (mode & (int)LoadMode::overrideChanges);

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
				.overridingObjects = overrideChanges
			};

           /* double progressToTotal = 0;
            if (progress)
            {
                progressToTotal = 1.0 - progress->getProgress();
                progress->setComment("Loading "+std::to_string(jsons.size()) + " objects");
            }
            double deltaProgress = (progressToTotal*0.5) / (jsons.size()+1);
            */

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
            replaceObjs.reserve(jsonCount);
            loadedObjects.reserve(jsonCount);

            double factor = 1 / (double)jsonCount;
            for (size_t i = 0; i < jsons.size(); ++i)
            {
                JDObjectManager::ManagedLoadMisc loaderMisc;
                bool loaded = false;
#if JD_ACTIVE_JSON == JD_JSON_QT
                const QJsonObject& json = jsons[i];
                if (Utilities::JDSerializable::getJsonValue(json, loaderMisc.id, JDObjectInterface::s_tag_objID))
                {
                    loaded = true;
                }
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
              
                if(!jsons[i].holds<JsonObject>())
				{
					JD_CONSOLE_FUNCTION("Json data is not an object: \"" << jsons[i] << "\"");
					success = false;
					continue;
				}
                const JsonObject& json = jsons[i].get<JsonObject>();
                
                if (json.contains(JDObjectInterface::s_tag_objID))
                {
#if JD_ACTIVE_JSON == JD_JSON_INTERNAL
                    loaderMisc.id = json.at(JDObjectInterface::s_tag_objID).get<int>();
#endif

                    loaded = true;
                }
				
#endif
                if (!loaded)
                {
#if JD_ACTIVE_JSON == JD_USE_QJSON
                    JD_CONSOLE_FUNCTION("Objet has incomplete data. Key: \""
                        << JDObjectInterface::s_tag_objID.toStdString() << "\" is missed\n");
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
                    JD_CONSOLE_FUNCTION("Objet has incomplete data. Key: \""
                        << JDObjectInterface::s_tag_objID << "\" is missed\n"
                        << "Object: \"" << json << "\"\n");
#endif
                    success = false;
                    continue;
                }

                JDObjectManager *manager = getObjectManager_internal(loaderMisc.id);

                JDObjectManager::ManagedLoadStatus status = JDObjectManager::managedLoad(
                    json, manager, loaderContainers, loadMode, loaderMisc);

                if(status != JDObjectManager::ManagedLoadStatus::success)
				{
					success = false;
                    JD_CONSOLE_FUNCTION("Failed to load object with ID: " << loaderMisc.id << " Error: \""
                        << JDObjectManager::managedLoadStatusToString(status) << "\"\n");
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
                   // if (progress) progress->addProgress(dProgress);
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
                success &= removeObject_internal(removedObjs);
                if (progress)
                {
                    progress->addProgress(0.1);
                    ++counter;
                }
            }
            JD_GENERAL_PROFILING_END_BLOCK;

            JD_GENERAL_PROFILING_BLOCK("Replace objects", JD_COLOR_STAGE_3);
            if (modeChangedObjects && !overrideChanges && changedPairs.size())
            {
                if(progress)
                    progress->setComment("Replace " + std::to_string(replaceObjs.size()) + " objects");
                replaceObject_internal(replaceObjs);
                if (progress)
                {
                    progress->addProgress(0.1);
                    ++counter;
                }
            }
            JD_GENERAL_PROFILING_END_BLOCK;

            JD_GENERAL_PROFILING_BLOCK("Add new objects", JD_COLOR_STAGE_3);
            if (modeNewObjects && newObjIDs.size())
            {
                if (progress)
                    progress->setComment("Add " + std::to_string(newObjIDs.size()) + " new objects");
                success &= packAndAddObject_internal(newObjIDs, newObjInstances);
                if (progress)
                {
                    progress->addProgress(0.1);
                    ++counter;
                }
                
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