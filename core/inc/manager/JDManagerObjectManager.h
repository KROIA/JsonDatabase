#pragma once

#include "JD_base.h"
#include "JDDeclaration.h"
#include "object/JDObjectContainer.h"
#include "utilities/JDObjectIDDomain.h"
#include "JDObjectLocker.h"

#include <vector>
#include <mutex>


#include <json/JsonValue.h>


namespace JsonDatabase
{
    namespace Internal
    {
        class JSONDATABASE_EXPORT JDManagerObjectManager
        {
            friend class JDManager;
        protected:
			JDManagerObjectManager(JDManager& manager, std::mutex &mtx);
            virtual ~JDManagerObjectManager();
            bool setup();
        public:
            
            void setDomainName(const std::string& name);

            template<typename T, typename... Args>
            std::shared_ptr<T> createInstance(Args&&... args);

            template<typename T>
            std::shared_ptr<T> createDeepClone(const std::shared_ptr<T> &source);
            template<typename T>
            std::shared_ptr<T> createShallowClone(const std::shared_ptr<T>& source);

            template<typename T>

            std::shared_ptr<T> createClone(const std::shared_ptr<T>& source, const JsonValue& data);

            
            bool addObject(JDObject obj);
            bool addObject(const std::vector<JDObject>& objList);

            JDObject replaceObject(JDObject obj);
            std::vector<JDObject> replaceObjects(const std::vector<JDObject>& objList);
           
            bool removeObject(JDObject obj);
            bool removeObjects(const std::vector<JDObject>& objList);
            template<typename T>
            bool removeObjects();
            //template<typename T>
            //bool deleteObjects();
            
            template<typename T>
            size_t getObjectCount() const;
            size_t getObjectCount() const;
            
            bool exists(JDObject obj) const;
            bool exists(const JDObjectIDptr& id) const;

            template<typename T>
            std::shared_ptr<T> getObject(const JDObjectIDptr& id);
            JDObject getObject(const JDObjectIDptr& id);
            template<typename T>
            std::shared_ptr<T> getObject(const JDObjectID::IDType& id);
            JDObject getObject(const JDObjectID::IDType& id);
            template<typename T>
            std::vector<std::shared_ptr<T>> getObjects() const;
            std::vector<JDObject> getObjects() const;

            void clearObjects();

            // Object locker interface
            bool lockObject(const JDObject& obj, JDObjectLocker::Error& err);
            bool unlockObject(const JDObject& obj, JDObjectLocker::Error& err);
            bool unlockAllObjs(JDObjectLocker::Error& err);
            bool isObjectLocked(const JDObject& obj, JDObjectLocker::Error& err) const;
            bool isObjectLockedByMe(const JDObject& obj, JDObjectLocker::Error& err) const;
            bool isObjectLockedByOther(const JDObject& obj, JDObjectLocker::Error& err) const;
            bool getLockedObjects(std::vector<JDObjectLocker::LockData>& lockedObjectsOut, JDObjectLocker::Error& err) const;
            int removeInactiveObjectLocks() const;




        protected:
            void onDatabasePathChange(const std::string& oldPath, const std::string& newPath);

            bool objectIDIsValid(const JDObjectIDptr& id) const;
            bool objectIDIsValid(const JDObject& obj) const;

            bool packAndAddObject_internal(const JDObject& obj);
            bool packAndAddObject_internal(const JDObject& obj, const JDObjectID::IDType& presetID);
            bool packAndAddObject_internal(const std::vector<JDObject> &objs);
            bool packAndAddObject_internal(const std::vector<JDObjectID::IDType> &ids, const std::vector<JDObject>& objs);

            JDObject replaceObject_internal(const JDObject& obj);
            void replaceObject_internal(const std::vector<JDObject>& objs);
            bool removeObject_internal(const JDObject& obj);
            bool removeObject_internal(const std::vector<JDObject>& objs);
            bool exists_internal(const JDObject& obj) const;
            bool exists_internal(const JDObjectIDptr& id) const;

            JDObject getObject_internal(const JDObjectIDptr& id);
            JDObject getObject_internal(const JDObjectID::IDType& id);
            std::vector<JDObject> getObjects_internal() const;
            JDObjectManager* getObjectManager_internal(const JDObjectIDptr& id);
            JDObjectManager* getObjectManager_internal(const JDObjectID::IDType& id);
            const std::vector<JDObjectManager*>& getObjectManagers_internal() const;
            void clearObjects_internal();

            bool loadObjectFromJson_internal(const JsonObject& json, const JDObject& obj);
            bool loadObjectsFromJson_internal(const JsonArray& jsons, int mode, Internal::WorkProgress* progress,
                std::vector<JDObject>& overridingObjs,
                std::vector<JDObjectID::IDType>& newObjIDs,
                std::vector<JDObject>& newObjInstances,
                std::vector<JDObject>& removedObjs,
                std::vector<JDObjectPair>& changedPairs);

            void update();

            
        private:
            JDManager& m_manager;
            JDObjectIDDomain m_idDomain;
            std::mutex &m_mutex;

            mutable std::mutex m_objsMutex;
            JDObjectContainer m_objs;
            Internal::JDObjectLocker m_objLocker;
        };


        template<typename T, typename... Args>
        std::shared_ptr<T> JDManagerObjectManager::createInstance(Args&&... args)
        {
            T* instanceRaw = new T(std::forward<Args>(args)...);
            std::shared_ptr<T> instance(instanceRaw);

            newObjectInstantiated_internal(instance);
            addObject(instance);

            return instance;
        }

        template<typename T>
        std::shared_ptr<T> JDManagerObjectManager::createDeepClone(const std::shared_ptr<T>& source)
        {
            T* cloned = dynamic_cast<T*>(source->deepClone_internal());
			std::shared_ptr<T> clone(cloned);

			//newObjectInstantiated_internal(clone);

			return clone;
        }
        template<typename T>
        std::shared_ptr<T> JDManagerObjectManager::createShallowClone(const std::shared_ptr<T>& source)
        {
            T* cloned = dynamic_cast<T*>(source->shallowClone_internal());
			std::shared_ptr<T> clone(cloned);

			//newObjectInstantiated_internal(clone);

			return clone;
        }
        
        template<typename T>
        std::shared_ptr<T> JDManagerObjectManager::createClone(const std::shared_ptr<T>& source, const JsonValue& data)
        {
            T* cloned = dynamic_cast<T*>(source->shallowClone_internal());
            cloned->loadInternal(data);
			std::shared_ptr<T> clone(cloned);

			return clone;

        }



        template<typename T>
        bool JDManagerObjectManager::removeObjects()
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
            std::vector<T*> toRemove = getObjects<T>();
            for (auto obj : toRemove)
            {
                m_objs.removeObject(obj);
            }
            return true;
        }


        template<typename T>
        std::size_t JDManagerObjectManager::getObjectCount() const
        {
            size_t c = 0;
            for (auto& o : m_objs)
            {
                if (dynamic_cast<T*>(o.get()))
                    ++c;
            }
            return c;
        }

        template<typename T>
        std::shared_ptr<T> JDManagerObjectManager::getObject(const JDObjectIDptr& id)
        {
            const JDObject & obj = getObject(id);
            std::shared_ptr<T> casted = std::dynamic_pointer_cast<T>(obj);
            return casted;
        }

        template<typename T>
        std::shared_ptr<T> JDManagerObjectManager::getObject(const JDObjectID::IDType& id)
        {
            const JDObject& obj = getObject(id);
            std::shared_ptr<T> casted = std::dynamic_pointer_cast<T>(obj);
            return casted;
        }

        template<typename T>
        std::vector<std::shared_ptr<T>> JDManagerObjectManager::getObjects() const
        {
            std::vector<std::shared_ptr<T>> list;
            list.reserve(m_objs.size());
            for (auto& p : m_objs)
            {
                std::shared_ptr<T> obj = std::dynamic_pointer_cast<T>(p);
                if (obj)
                    list.push_back(obj);
            }
            return list;
        }
    }
}