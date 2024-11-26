#pragma once

#include "JsonDatabase_base.h"
#include "JsonDatabase_Declaration.h"
#include "utilities/ErrorCodes.h"
#include "object/JDObjectContainer.h"
#include "utilities/JDObjectIDDomain.h"
#include "utilities/JDUniqueMutexLock.h"
#include "JDObjectLocker.h"

#include <vector>
#include <mutex>

#include "Logger.h"

#include <json/JsonValue.h>


namespace JsonDatabase
{
    namespace Internal
    {
        class JSON_DATABASE_EXPORT JDManagerObjectManager
        {
            friend class JDManager;
        protected:
			JDManagerObjectManager(
                JDManager& manager, 
                std::mutex &mtx);
            virtual ~JDManagerObjectManager();
            void setParentLogger(Log::LogObject* parentLogger);
            bool setup();
            bool stop();

            /**
             * @brief 
			 * Sets the domain for the object IDs
			 * The same ID can only exist once in the same domain
             * @param name 
             */
            void setDomainName(const std::string& name);
            
        public:
            
           
            /**
             * @brief 
			 * Instantiates a new object of type T and adds it to the database
             * @note This function does lock the database mutex
             * @tparam T type of 
             * @tparam ...Args 
			 * @param ...args Arguments to pass to the constructor of the derived JDObject
			 * @return the shared pointer to the new object. Returns nullptr if the object could not be added to the database
             */
            template<typename T, typename... Args>
            std::shared_ptr<T> createInstance(Args&&... args);

            /**
             * @brief 
			 * Creates a deep clone of the source object
			 * @see JDObjectInterface::deepClone()
             * @tparam T 
             * @param source object to clone from
			 * @return cloned instance of the object
             */
            template<typename T>
            std::shared_ptr<T> createDeepClone(const std::shared_ptr<T> &source);

            /**
             * @brief 
			 * Creates a shallow clone of the source object
			 * @see JDObjectInterface::shallowClone()
             * @tparam T 
             * @param source object to clone from
             * @return cloned instance of the object
             */
            template<typename T>
            std::shared_ptr<T> createShallowClone(const std::shared_ptr<T>& source);

            /**
             * @brief
             * Creates a shallow clone of the source object and 
			 * loads the data from the given JsonValue
             * @see JDObjectInterface::shallowClone()
             * @tparam T
             * @param source object to clone from
             * @return cloned instance of the object
             */
            template<typename T>
            std::shared_ptr<T> createClone(const std::shared_ptr<T>& source, const JsonValue& data);

            
            /**
             * @brief 
			 * Adds an object to the database
             * @note This function does lock the database mutex
             * @param obj 
			 * @return true if the object was added successfully, otherwise false
             */
            bool addObject(JDObject obj);

            /**
             * @brief 
			 * Adds a list of objects to the database
             * @note This function does lock the database mutex
             * @param objList 
			 * @return true if all objects were added successfully, otherwise false
             */
            bool addObject(const std::vector<JDObject>& objList);

            /**
             * @brief 
			 * Removes an object from the database
             * @note This function does lock the database mutex
             * @note This function does not delete the object
             * @param obj 
			 * @return true if the object was removed successfully, otherwise false
             */
            bool removeObject(JDObject obj);

            /**
             * @brief 
			 * Removes a list of objects from the database
             * @note This function does lock the database mutex
             * @note This function does not delete the objects
             * @param objList 
             * @return 
             */
            bool removeObjects(const std::vector<JDObject>& objList);

            /**
             * @brief 
			 * Removes all objects of type T from the database
             * @note This function does lock the database mutex
             * @note This function does not delete the objects
             * @tparam T 
			 * @return true if all objects were removed successfully, otherwise false
             */
            template<typename T>
            bool removeObjects();
            
            /**
             * @brief 
			 * Counts the number of objects of type T in the database
             * @note This function does lock the database mutex
             * @tparam T 
			 * @return the number of objects of type T in the database
             */
            template<typename T>
            size_t getObjectCount() const;

            /**
             * @brief 
			 * Counts the number of objects in the database
             * @note This function does lock the database mutex
			 * @return the number of objects in the database
             */
            size_t getObjectCount() const;
            
            /**
             * @brief 
			 * Checks if the object exists in the database
             * @note This function does lock the database mutex
             * @param obj to search in the database
			 * @return true if the object exists in the database, otherwise false
             */
            bool exists(JDObject obj) const;

            /**
             * @brief 
			 * Checks if all objects exist in the database
             * @note This function does lock the database mutex
             * @param objs to search in the database
			 * @return true if all objects exist in the database, otherwise false
             */
            bool exists(const std::vector<JDObject> &objs) const;

            /**
             * @brief 
			 * Checks if a object with the given ID exists in the database
             * @note This function does lock the database mutex
             * @param id object from the target object
			 * @return true if the object exists in the database, otherwise false
             */
            bool exists(const JDObjectIDptr& id) const;

            /**
             * @brief 
			 * Gets the object with the given ID and casts it to type T
             * @note This function does lock the database mutex
             * @tparam T 
             * @param id to search for the object 
			 * @return ptr to the object if it exist and is compatible to type T, otherwise nullptr
             */
            template<typename T>
            std::shared_ptr<T> getObject(const JDObjectIDptr& id);

            /**
             * @brief 
			 * Gets the object with the given ID
             * @note This function does lock the database mutex
			 * @param id to search for the object
			 * @return ptr to the object if it exist, otherwise nullptr
             */
            JDObject getObject(const JDObjectIDptr& id);

            /**
             * @brief 
			 * Gets the object with the given ID and casts it to type T
             * @note This function does lock the database mutex
             * @tparam T 
			 * @param id to search for the object
			 * @return ptr to the object if it exist and is compatible to type T, otherwise nullptr
             */
            template<typename T>
            std::shared_ptr<T> getObject(const JDObjectID::IDType& id);

            /**
             * @brief 
			 * Gets the object with the given ID
             * @note This function does lock the database mutex
             * @param id 
			 * @return ptr to the object if it exist, otherwise nullptr
             */
            JDObject getObject(const JDObjectID::IDType& id);

            /**
             * @brief 
             * Gets a list of all objects of the given type T
             * @note This function does lock the database mutex
             * @tparam T 
			 * @return list of all objects of type T
             */
            template<typename T>
            std::vector<std::shared_ptr<T>> getObjects() const;

            /**
             * @brief 
			 * Gets a list of all objects in the database
             * @note This function does lock the database mutex
			 * @return list of all objects in the database
             */
            std::vector<JDObject> getObjects() const;

            /**
             * @brief 
             * Removes all objects from this database
             * @note This function does lock the database mutex
             * @note This function does not delete the objects
             */
            void clearObjects();

            // Object locker interface

            /**
             * @brief 
			 * Try to lock the object
             * @note This function does lock the database mutex
             * @param obj to lock
			 * @param err to store the error message if the object could not be locked
			 * @return true if the object was locked successfully, otherwise false
             */
            bool lockObject(const JDObject& obj, Error& err);

			/**
			 * @brief 
			 * Try to lock the objects
             * @note This function does lock the database mutex
			 * @param objs list of objects to lock
			 * @param errors list of errors for each object, can be empty when calling the function
			 * @return true if all objects were locked successfully, otherwise false in that case check the error list
			 */
			bool lockObjects(const std::vector<JDObject>& objs, std::vector<Error>& errors);

            /**
             * @brief 
			 * Try to unlock the object
             * @note This function does lock the database mutex
             * @param obj to unlock
			 * @param err to store the error message if the object could not be unlocked
             * @return 
             */
            bool unlockObject(const JDObject& obj, Error& err);

            /**
             * @brief 
			 * Try to unlock the objects
             * @note This function does lock the database mutex
			 * @param id of the object that should be unlocked
			 * @param err to store the error message if the object could not be unlocked
			 * @return true if the object was unlocked successfully, otherwise false
             */
            bool unlockObject(const JDObjectID::IDType& id, Error& err);

			/**
			 * @brief 
			 * Try to unlock the objects
             * @note This function does lock the database mutex
			 * @param objs list of objects to unlock
			 * @param errors list of errors for each object, can be empty when calling the function
			 * @return true if all objects were unlocked successfully, otherwise false in that case check the error list
			 */
			bool unlockObjects(const std::vector<JDObject>& objs, std::vector<Error>& errors);

            /**
             * @brief 
			 * Try to lock all objects in the database
             * @note This function does lock the database mutex
			 * @param err to store a summary error if any object could not be locked
			 * @return true if all objects were locked successfully, otherwise false
             */
            bool lockAllObjs(Error& err);

            /**
             * @brief 
			 * Try to unlock all objects in the database
             * @note This function does lock the database mutex
			 * @param err to store a summary error if any object could not be unlocked
			 * @return true if all objects were unlocked successfully, otherwise false
             */
            bool unlockAllObjs(Error& err);

            /**
             * @brief 
			 * Check if the object is locked
             * @note This function does lock the database mutex
             * @param obj object to check
			 * @param err to store the error message if the object could not be checked
             * @return true if the object is locked, otherwise false
             */
            bool isObjectLocked(const JDObject& obj, Error& err) const;

            /**
             * @brief 
			 * Check if the object is locked by the current session
             * @note This function does lock the database mutex
			 * @param obj object to check
			 * @param err to store the error message if the object could not be checked
			 * @return true if the object is locked by the current session, otherwise false
             */
            bool isObjectLockedByMe(const JDObject& obj, Error& err) const;

            /**
             * @brief 
			 * Check if the object is locked by another session
             * @note This function does lock the database mutex
			 * @param obj object to check
			 * @param err to store the error message if the object could not be checked
			 * @return true if the object is locked by another session, otherwise false
             */
            bool isObjectLockedByOther(const JDObject& obj, Error& err) const;

            /**
             * @brief 
             * Gets the LockData for all locked objects.
             * 
             * Usecases:
             *   - Use this function if you have to check many objects for they're lock status.
             *     Calling this function once is much more efficient than reading the locks individualy
             * @note This function does lock the database mutex
             * @param lockedObjectsOut 
			 * @param err to store the error message if the object could not be checked
			 * @return true if the locks could be retrieved successfully, otherwise false
             */
            bool getObjectLocks(std::vector<JDObjectLocker::LockData>& lockedObjectsOut, Error& err) const;
           
            /**
             * @brief 
             * Gets a list of LockData of locked objects that are locked by the <user>
             * The user is identified by its sessionID
             * @note This function does lock the database mutex
             * @param user to filter for locked objects
             * @param lockedObjectsOut that contains all locks owned by the specific <user>
			 * @param err to store the error message if the object could not be checked
			 * @return true if the locks could be retrieved successfully, otherwise false
             */
            bool getObjectLocksByUser(const Utilities::JDUser &user, std::vector<JDObjectLocker::LockData>& lockedObjectsOut, Error& err) const;
			
            /**
             * @brief 
             * Helper struct to combine a Object and its LockData
             */
            struct LockedObject
            {
                JDObject obj;
				JDObjectLocker::LockData lockData;
            };

            /**
             * @brief 
			 * Gets a list of all locked objects and they're LockData
             * @note This function does lock the database mutex
             * @param lockedObjectsOut list of locked objects
			 * @param err to store the error message if the object could not be checked
			 * @return true if the locks could be retrieved successfully, otherwise false
             */
            bool getLockedObjects(std::vector<LockedObject>& lockedObjectsOut, Error& err) const;

            /**
             * @brief 
             * Gets a list of all locked objects
             * @note This function does lock the database mutex
             * @param lockedObjectsOut list of locked objects
			 * @param err to store the error message if the object could not be checked
			 * @return true if the locks could be retrieved successfully, otherwise false
             */
            bool getLockedObjects(std::vector<JDObject>& lockedObjectsOut, Error& err) const;

			/**
			 * @brief 
			 * Gets a list of all locked objects that are locked by the <user>
             * @note This function does lock the database mutex
			 * @param user to filter for locked objects
			 * @param lockedObjectsOut 
			 * @param err 
			 * @return 
			 */
			bool getLockedObjects(const Utilities::JDUser& user, std::vector<JDObject>& lockedObjectsOut, Error& err) const;
           
            /**
             * @brief 
			 * Gets the owner of the lock for the object
             * @note This function does lock the database mutex
             * @param obj that is locked
             * @param userOut user that owns the lock
             * @param err to store if something went wrong
			 * @return true if the check was successful, otherwise false -> check err
             */
            bool getLockOwner(const JDObject& obj, Utilities::JDUser& userOut, Error& err) const;
			
            /**
             * @brief 
             * Gets the lock data for the object
             * @note This function does lock the database mutex
             * @param obj that is locked
             * @param lockDataOut LockData for the locked object
             * @param err to store if something went wrong
             * @return true if the ckeck was successfull, otherwise false -> check err
             */
            bool getLockData(const JDObject& obj, JDObjectLocker::LockData& lockDataOut, Error& err) const;
            
            /**
             * @brief 
             * Try's to delete all object locks. Locks can't be deleted if 
             * another session holds them.
             * In case of a crash the lock files will still be available.
             * To automaticly clean up, this function is called periodicly
             * to try to delete the lock files.
             * @note This function does lock the database mutex
             * @return the amount of deleted locks.
             */
            int removeInactiveObjectLocks() const;




        protected:
            void onDatabasePathChange(const std::string& oldPath, const std::string& newPath);

            bool objectIDIsValid(const JDObjectIDptr& id) const;
            bool objectIDIsValid(const JDObject& obj) const;

            bool packAndAddObject_internal(const JDObject& obj);
            bool packAndAddObject_internal(const JDObject& obj, const JDObjectID::IDType& presetID);
            bool packAndAddObject_internal(const std::vector<JDObject> &objs);
            bool packAndAddObject_internal(const std::vector<JDObjectID::IDType> &ids, const std::vector<JDObject>& objs);

            //JDObject replaceObject_internal(const JDObject& obj);
            //void replaceObject_internal(const std::vector<JDObject>& objs);
            bool removeObject_internal(const JDObject& obj, bool doSave = true);
            bool removeObject_internal(const std::vector<JDObject>& objs, bool doSave = true);
            bool exists_internal(const JDObject& obj) const;
            bool exists_internal(const std::vector<JDObject>& objs) const;
            bool exists_internal(const JDObjectIDptr& id) const;

            JDObject getObject_internal(const JDObjectIDptr& id) const;
            JDObject getObject_internal(const JDObjectID::IDType& id) const;
            std::vector<JDObject> getObjects_internal() const;
            JDObjectManager* getObjectManager_internal(const JDObjectIDptr& id);
            JDObjectManager* getObjectManager_internal(const JDObjectID::IDType& id);
            const std::vector<JDObjectManager*>& getObjectManagers_internal() const;
            void clearObjects_internal();

            bool getLockedObjects_internal(std::vector<LockedObject>& lockedObjectsOut, Error& err) const;
            bool getLockedObjects_internal(std::vector<JDObject>& lockedObjectsOut, Error& err) const;
            bool getObjectLocksByUser_internal(const Utilities::JDUser& user, std::vector<JDObjectLocker::LockData>& lockedObjectsOut, Error& err) const;
            bool getLockedObjects_internal(const Utilities::JDUser& user, std::vector<JDObject>& lockedObjectsOut, Error& err) const;
            bool getLockOwner_internal(const JDObject& obj, Utilities::JDUser& userOut, Error& err) const;
            bool getLockData_internal(const JDObject& obj, JDObjectLocker::LockData& lockDataOut, Error& err) const;


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

            Log::LogObject* m_logger = nullptr;
        };


        template<typename T, typename... Args>
        std::shared_ptr<T> JDManagerObjectManager::createInstance(Args&&... args)
        {
            // Check if T is a derived type of JDObjectInterface
			static_assert(std::is_base_of<JDObjectInterface, T>::value, "T must be derived from JDObjectInterface");
            T* instanceRaw = new T(std::forward<Args>(args)...);
            std::shared_ptr<T> instance(instanceRaw);
            if (addObject(instance))
                return instance;

            return nullptr;
        }

        template<typename T>
        std::shared_ptr<T> JDManagerObjectManager::createDeepClone(const std::shared_ptr<T>& source)
        {
            // Check if T is a derived type of JDObjectInterface
            static_assert(std::is_base_of<JDObjectInterface, T>::value, "T must be derived from JDObjectInterface");
            T* cloned = dynamic_cast<T*>(source->deepClone_internal());
			std::shared_ptr<T> clone(cloned);
			return clone;
        }
        template<typename T>
        std::shared_ptr<T> JDManagerObjectManager::createShallowClone(const std::shared_ptr<T>& source)
        {
            // Check if T is a derived type of JDObjectInterface
            static_assert(std::is_base_of<JDObjectInterface, T>::value, "T must be derived from JDObjectInterface");
            T* cloned = dynamic_cast<T*>(source->shallowClone_internal());
			std::shared_ptr<T> clone(cloned);
			return clone;
        }
        
        template<typename T>
        std::shared_ptr<T> JDManagerObjectManager::createClone(const std::shared_ptr<T>& source, const JsonValue& data)
        {
            // Check if T is a derived type of JDObjectInterface
            static_assert(std::is_base_of<JDObjectInterface, T>::value, "T must be derived from JDObjectInterface");
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
			return removeObjects(toRemove);
        }


        template<typename T>
        std::size_t JDManagerObjectManager::getObjectCount() const
        {
            JDM_UNIQUE_LOCK_P_M(m_objsMutex);
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
            JDM_UNIQUE_LOCK_P_M(m_objsMutex);
            std::vector<std::shared_ptr<T>> list;
            list.reserve(m_objs.size());
            for (auto& p : m_objs)
            {
                std::shared_ptr<T> obj = std::dynamic_pointer_cast<T>(p->getObject());
                if (obj)
                    list.push_back(obj);
            }
            return list;
        }
    }
}
