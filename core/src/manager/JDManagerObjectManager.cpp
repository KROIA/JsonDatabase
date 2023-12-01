#include "manager/JDManagerObjectManager.h"
#include "utilities/JDUniqueMutexLock.h"
#include "object/JDObjectInterface.h"
#include "manager/async/WorkProgress.h"
#include "utilities/JsonUtilities.h"

namespace JsonDatabase
{
    namespace Internal
    {
        JDManagerObjectManager::JDManagerObjectManager(const std::string &domainName, std::mutex& mtx)
            : m_mutex(mtx)
            , m_idDomain(domainName)
        {

        }
        JDManagerObjectManager::~JDManagerObjectManager()
        {

        }
        bool JDManagerObjectManager::setup()
        {
            return true;
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

        /*
          -----------------------------------------------------------------------------------------------
          ------------------ I N T E R N A L ------------------------------------------------------------
          -----------------------------------------------------------------------------------------------
        */
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
            if (m_objs.exists(obj) || !obj.get())
                return false; // Object already added
            if (obj->isManaged())
                return false; // Object already managed
            bool success;
            JDObjectIDptr id = m_idDomain.getNewID(presetID, success);
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
            m_objs.reserve(m_objs.size() + objs.size());
            bool success = true;
            for (size_t i = 0; i < objs.size(); ++i)
				success &= packAndAddObject_internal(objs[i]);
			return success;
        }
        bool JDManagerObjectManager::packAndAddObject_internal(const std::vector<std::pair<JDObjectID::IDType, JDObject>>& objs)
        {
            m_objs.reserve(m_objs.size() + objs.size());
            bool success = true;
            for (size_t i = 0; i < objs.size(); ++i)
                success &= packAndAddObject_internal(objs[i].second, objs[i].first);
            return success;
        }
        /*bool JDManagerObjectManager::addObject_internal(JDObjectManager* obj)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            //obj->m_onDelete.connectSlot(this, &JDManagerObjectManager::onObjectGotDeleted);
            return m_objs.addObject(obj);
        }
        bool JDManagerObjectManager::addObject_internal(const std::vector<JDObjectManager*>& objs)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            //for(auto obj : objs)
            //    obj->m_onDelete.connectSlot(this, &JDManagerObjectManager::onObjectGotDeleted);
            return m_objs.addObject(objs);
        }*/
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
            //for (auto obj : objs)
			//	obj->m_onDelete.disconnectSlot(this, &JDManagerObjectManager::onObjectGotDeleted);
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
        std::vector<JDObjectManager*> JDManagerObjectManager::getObjectManagers_internal() const
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            return m_objs.getAllObjects();
        }
        void JDManagerObjectManager::clearObjects_internal()
        {
            m_objs.clear();
        }


#ifdef JD_USE_QJSON
        bool JDManagerObjectManager::loadObjectsFromJson_internal(const std::vector<QJsonObject>& jsons, int mode, Internal::WorkProgress* progress,
            bool hasOverrideChangeFromDatabaseSlots,
            bool hasChangeFromDatabaseSlots,
            bool hasObjectAddedToDatabaseSlots,
            bool hasObjectRemovedFromDatabaseSlots,
            std::vector<JDObject>& overridingObjs,
            std::vector<std::pair<JDObjectID::IDType, JDObject>>& newObjs,
            std::vector<JDObject>& removedObjs,
            std::vector<JDObjectPair>& changedPairs)
#else
        bool JDManagerObjectManager::loadObjectsFromJson_internal(const JsonArray& jsons, int mode, Internal::WorkProgress* progress,
            bool hasOverrideChangeFromDatabaseSlots,
            bool hasChangeFromDatabaseSlots,
            bool hasObjectAddedToDatabaseSlots,
            bool hasObjectRemovedFromDatabaseSlots,
            std::vector<JDObject> &overridingObjs,
            std::vector<std::pair<JDObjectID::IDType, JDObject>> &newObjs,
            std::vector<JDObject> &removedObjs,
            std::vector<JDObjectPair> &changedPairs)
#endif
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);

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
                .newObjs = newObjs,
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
            



            for (size_t i = 0; i < jsons.size(); ++i)
            {
                JDObjectManager::ManagedLoadMisc loaderMisc;
#ifdef JD_USE_QJSON
                const QJsonObject& json = jsons[i];
                if (!json.getInt(id, JDObjectInterface::s_tag_objID))
#else
                const JsonValue& json = jsons[i];
                if (!jsons[i].getInt(loaderMisc.id, JDObjectInterface::s_tag_objID))
#endif
                {
#ifndef JD_USE_QJSON
                    JD_CONSOLE_FUNCTION("Objet has incomplete data. Key: \""
                        << JDObjectInterface::s_tag_objID << "\" is missed\n"
                        << "Object: \"" << json << "\"\n");
#else
                    JD_CONSOLE_FUNCTION("Objet has incomplete data. Key: \""
                        << JDObjectInterface::s_tag_objID.toStdString() << "\" is missed\n");
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
            }

            // Find new added objects
            if (modeRemovedObjects)
            {
                std::vector<JDObject> allObjs = getObjects_internal();

                removedObjs.reserve(allObjs.size());
                for (auto obj : allObjs)
                {
                    if (loadedObjects.find(obj) != loadedObjects.end())
                        continue;

                    removedObjs.emplace_back(obj);
                nextObj:;
                   // if (progress) progress->addProgress(dProgress);
                }
            }

            /*struct Pair
            {
                JDObject objOriginal;
                JDObject obj;
#ifdef JD_USE_QJSON
                QJsonObject& json;
#else
                const JsonValue& json;
#endif
            };

            std::vector< Pair> pairs;
            pairs.reserve(jsons.size());
            std::vector< Pair> newObjectPairs;
            newObjectPairs.reserve(jsons.size());
            {
                JD_GENERAL_PROFILING_BLOCK("Match objects with json data", JD_COLOR_STAGE_2);
                if (progress) progress->setComment("Matching objects with json data");
                for (size_t i = 0; i < jsons.size(); ++i)
                {
                    //JDObjectID ID;
                    JDObjectID::IDType id;
                    JD_GENERAL_PROFILING_NONSCOPED_BLOCK("Get object ID from json", JD_COLOR_STAGE_3);
#ifdef JD_USE_QJSON
                    if (!JDSerializable::getJsonValue(jsons[i], id, JDObjectInterface::s_tag_objID))
#else

                    if (!jsons[i].getInt(id, JDObjectInterface::s_tag_objID))
#endif
                    {
#ifdef JD_USE_QJSON
                        JD_CONSOLE("bool JDManager::loadObjects_internal(mode=\"" << getLoadModeStr(mode)
                            << "\") Object with no ID found: "
                            << QJsonValue(jsons[i]).toString().toStdString()
                            << "\n");
#else
                        JD_CONSOLE("bool JDManager::loadObjects_internal(mode=\"" << getLoadModeStr(mode)
                            << "\") Object with no ID found: "
                            << jsons[i].toString()
                            << "\n");
#endif
                        success = false;
                    }
                    JD_GENERAL_PROFILING_END_BLOCK;

                    //JDObjectIDptr ID = JDManagerObjectManager::m_idDomain.getExistingID(id);

                    Pair p{ .objOriginal = nullptr, .obj = nullptr, .json = jsons[i] };
                    //if (JDManagerObjectManager::objectIDIsValid(id))
                    p.objOriginal = getObject_internal(id);
                    if (p.objOriginal)
                        pairs.emplace_back(p);
                    else
                        newObjectPairs.emplace_back(p);

                }
                if (progress) progress->addProgress(0.03);
            }
            if (!success)
            {
                return false;
            }

            double subProgress = 1;
            if (progress)
                subProgress = (1 - progress->getProgress()) / 3.0;


          // const bool hasOverrideChangeFromDatabaseSlots = m_signals.objectOverrideChangeFromDatabase.signal.getSlotCount();
          // const bool hasChangeFromDatabaseSlots = m_signals.objectChangedFromDatabase.signal.getSlotCount();
          // const bool hasObjectAddedToDatabase = m_signals.objectAddedToDatabase.signal.getSlotCount();
          // const bool hasObjectRemovedFromDatabase = m_signals.objectRemovedFromDatabase.signal.getSlotCount();

             //std::vector<JDObject> overridingObjs;
             //std::vector<JDObject> newObjs;
             //std::vector<JDObject> removedObjs;
             //std::vector<JDObjectPair> changedPairs;

            

            std::unordered_map<JDObject, JDObject> loadedObjects;
            if (modeRemovedObjects) loadedObjects.reserve(jsons.size());

            if (modeChangedObjects && pairs.size() > 0)
            {
                double dProgress = subProgress / pairs.size();
                if (overrideChanges)
                {
                    JD_GENERAL_PROFILING_BLOCK("Deserialize objects override mode", JD_COLOR_STAGE_2);
                    if (progress) progress->setComment("Deserializing objects override mode");

                    overridingObjs.reserve(pairs.size());
                    // Loads the existing objects and overrides the data in the current object instance
                    for (Pair& pair : pairs)
                    {
                        if (hasOverrideChangeFromDatabaseSlots)
                        {
                            bool hasChanged = false;
                            success &= Internal::JsonUtilities::deserializeOverrideFromJson(pair.json, pair.objOriginal, hasChanged);
                            if (modeRemovedObjects) loadedObjects[pair.objOriginal] = pair.objOriginal;
                            if (hasChanged)
                            {
                                // The loaded object is not equal to the original object
                                overridingObjs.push_back(pair.objOriginal);
                            }
                        }
                        else
                            success &= Internal::JsonUtilities::deserializeOverrideFromJson(pair.json, pair.objOriginal);
                        pair.obj = pair.objOriginal;
                        if (progress) progress->addProgress(dProgress);
                    }

                }
                else
                {
                    JD_GENERAL_PROFILING_BLOCK("Deserialize objects reinstatiation mode", JD_COLOR_STAGE_2);
                    if (progress) progress->setComment("Deserializing objects reinstatiation mode");

                    changedPairs.reserve(pairs.size());
                    replaceObjs.reserve(pairs.size());
                    // Loads the existing objects and creates a new object instance if the data has changed
                    for (Pair& pair : pairs)
                    {
                        success &= Internal::JsonUtilities::deserializeJson(
                            pair.json,
                            pair.objOriginal,
                            pair.obj,
                            JDManagerObjectManager::m_idDomain,
                            *this);
                        if (modeRemovedObjects) loadedObjects[pair.objOriginal] = pair.objOriginal;
                        if (pair.objOriginal != pair.obj)
                        {
                            // The loaded object is not equal to the original object
                            if (hasChangeFromDatabaseSlots)
                                changedPairs.emplace_back(std::move(JDObjectPair(pair.objOriginal, pair.obj)));
                            replaceObjs.emplace_back(pair.obj);
                            //replaceObject_internal(pair.obj);
                        }
                        if (progress) progress->addProgress(dProgress);
                    }
                }

            }
            else if (progress) progress->addProgress(subProgress);

            if (modeNewObjects && newObjectPairs.size() > 0)
            {
                JD_GENERAL_PROFILING_BLOCK("Deserialize and create new objects", JD_COLOR_STAGE_2);
                if (progress) progress->setComment("Deserializing and creating new objects");
                double dProgress = subProgress / newObjectPairs.size();


                newObjs.reserve(newObjectPairs.size());
                // Loads the new objects and creates a new object instance
                try
                {

                    for (Pair& pair : newObjectPairs)
                    {
                        success &= Internal::JsonUtilities::deserializeJson(
                            pair.json,
                            pair.objOriginal,
                            pair.obj,
                            JDManagerObjectManager::m_idDomain,
                            *this);
                        if (modeRemovedObjects) loadedObjects[pair.obj] = pair.obj;
                        // Add the new generated object to the database
                        //addObject_internal(pair.obj);
                        newObjs.emplace_back(pair.obj);
                        if (progress) progress->addProgress(dProgress);
                    }
                }
                catch (const std::exception& e)
                {
                    JD_CONSOLE_FUNCTION("Exception: " << e.what() << "\n");
                    //JDManagerFileSystem::unlockFile(error);
                    return false;
                }
                catch (...)
                {
                    JD_CONSOLE_FUNCTION("Exception unknown\n");
                    //JDManagerFileSystem::unlockFile(error);
                    return false;
                }

            }
            else if (progress) progress->addProgress(subProgress);

            if (modeRemovedObjects)
            {
                JD_GENERAL_PROFILING_BLOCK("Search for erased objects", JD_COLOR_STAGE_2);
                if (progress) progress->setComment("Searching for erased objects");
                std::vector<JDObject> allObjs = getObjects_internal();
                double dProgress = subProgress / (allObjs.size() + 1);

                removedObjs.reserve(allObjs.size());
                for (auto obj : allObjs)
                {
                    if (loadedObjects.find(obj) != loadedObjects.end())
                        continue;

                    if (hasObjectRemovedFromDatabase)
                        removedObjs.emplace_back(obj);


                nextObj:;
                    if (progress) progress->addProgress(dProgress);
                }

            }
            else if (progress) progress->addProgress(subProgress);
            //JDManagerFileSystem::unlockFile(error);
            */

            // Copy the data to the signals
            if (modeChangedObjects)
            {
                if (overrideChanges)
                {
                   // if (hasOverrideChangeFromDatabaseSlots && overridingObjs.size())
                   //     m_signals.objectOverrideChangeFromDatabase.addObjs(overridingObjs);
                }
                else
                {
                    if (changedPairs.size())
                    {
                        replaceObject_internal(replaceObjs);
                        //if (hasChangeFromDatabaseSlots)
                        //   m_signals.objectChangedFromDatabase.addPairs(changedPairs);
                    }
                }
            }
            if (modeRemovedObjects)
            {
                if (removedObjs.size())
                {
                    success &= removeObject_internal(removedObjs);
                    //if (hasObjectRemovedFromDatabase)
                    //    m_signals.objectRemovedFromDatabase.addObjs(removedObjs);
                }
            }
            if (modeNewObjects)
            {
                if (newObjs.size())
                {
                    success &= packAndAddObject_internal(newObjs);
                    //if (hasObjectAddedToDatabase)
                    //    m_signals.objectAddedToDatabase.addObjs(newObjs);
                }
            }
            return success;
        }


        void JDManagerObjectManager::update()
        {

        }

    }
}