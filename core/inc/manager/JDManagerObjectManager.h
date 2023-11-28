#pragma once

#include "JD_base.h"
#include "JDDeclaration.h"
#include "object/JDObjectContainer.h"
#include "utilities/JDObjectIDDomain.h"

#include <vector>
#include <mutex>

namespace JsonDatabase
{
    namespace Internal
    {
        class JSONDATABASE_EXPORT JDManagerObjectManager
        {
            //friend class JDManager;
        protected:
			JDManagerObjectManager(const std::string &domainName, std::mutex &mtx);
            virtual ~JDManagerObjectManager();
            bool setup();
        public:



            template<typename T, typename... Args>
            std::shared_ptr<T> createInstance(Args&&... args);

            template<typename T>
            std::shared_ptr<T> createClone(const T* source);

            template<typename T>
#ifdef JD_USE_QJSON
            std::shared_ptr<T> createClone(const T* source, const QJsonObject& data);
#else
            std::shared_ptr<T> createClone(const T* source, const JsonValue& data);
#endif

            template<typename T>
#ifdef JD_USE_QJSON
            std::shared_ptr<T> createClone(const T* source, const QJsonObject& data, const JDObjectIDptr &id);
#else
            std::shared_ptr<T> createClone(const T* source, const JsonValue& data, const JDObjectIDptr& id);
#endif
            
            bool addObject(const JDObject & obj);
            bool addObject(const std::vector<JDObject>& objList);

            const JDObject& replaceObject(const JDObject & obj);
            std::vector<JDObject> replaceObjects(const std::vector<JDObject>& objList);
           
            bool removeObject(const JDObject& obj);
            bool removeObjects(const std::vector<JDObject>& objList);
            template<typename T>
            bool removeObjects();
            template<typename T>
            
            bool deleteObjects();
            
            template<typename T>
            size_t getObjectCount() const;
            size_t getObjectCount() const;
            
            bool exists(const JDObject & obj) const;
            bool exists(const JDObjectIDptr& id) const;

            template<typename T>
            T* getObject(const JDObjectIDptr& id);
            const JDObject& getObject(const JDObjectIDptr& id);
            template<typename T>
            T* getObject(const JDObjectID::IDType& id);
            const JDObject& getObject(const JDObjectID::IDType& id);
            template<typename T>
            std::vector<T*> getObjects() const;
            const std::vector<JDObject>& getObjects() const;

            void clearObjects();
        protected:

            void newObjectInstantiated_internal(const JDObject& obj);

            bool addObject_internal(const JDObject& obj);
            bool addObject_internal(const std::vector<JDObject>& objs);
            const JDObject& replaceObject_internal(const JDObject& obj);
            void replaceObject_internal(const std::vector<JDObject>& objs);
            bool removeObject_internal(const JDObject& obj);
            bool removeObject_internal(const std::vector<JDObject>& objs);
            bool exists_internal(const JDObject& obj) const;
            bool exists_internal(const JDObjectIDptr& id) const;
            const JDObject& getObject_internal(const JDObjectIDptr& id);
            const JDObject& getObject_internal(const JDObjectID::IDType& id);
            const std::vector<JDObject>& getObjects_internal() const;
            void clearObjects_internal();

            void update();

            JDObjectIDDomain m_idDomain;
        private:
            std::mutex &m_mutex;

            mutable std::mutex m_objsMutex;
            JDObjectContainer m_objs;
            
        };


        template<typename T, typename... Args>
        std::shared_ptr<T> JDManagerObjectManager::createInstance(Args&&... args)
        {
            std::shared_ptr<T> instance(new T(std::forward<Args>(args)...));

            newObjectInstantiated_internal(instance);

            return instance;
        }

        template<typename T>
        std::shared_ptr<T> JDManagerObjectManager::createClone(const T* source)
        {
			std::shared_ptr<T> clone(dynamic_cast<T*>(source->clone_internal()));

			//newObjectInstantiated_internal(clone);

			return clone;
        }
        
        template<typename T>
#ifdef JD_USE_QJSON
        std::shared_ptr<T> JDManagerObjectManager::createClone(const T* source, const QJsonObject& data)
#else
        std::shared_ptr<T> JDManagerObjectManager::createClone(const T* source, const JsonValue& data)
#endif
        {
			std::shared_ptr<T> clone(dynamic_cast<T*>(source->clone_internal(data)));

			//newObjectInstantiated_internal(clone);

			return clone;

        }
        template<typename T>
#ifdef JD_USE_QJSON
        std::shared_ptr<T> JDManagerObjectManager::createClone(const T* source, const QJsonObject& data, const JDObjectIDptr& id)
#else
        std::shared_ptr<T> JDManagerObjectManager::createClone(const T* source, const JsonValue& data, const JDObjectIDptr& id)
#endif
        {
            std::shared_ptr<T> clone(dynamic_cast<T*>(source->clone_internal(data, id)));

            //newObjectInstantiated_internal(clone);

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
        bool JDManagerObjectManager::deleteObjects()
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
            std::vector<T*> toDelete = getObjects<T>();

            for (auto obj : toDelete)
            {
                m_objs.removeObject(obj);
                delete obj;
            }
            return true;
        }

        template<typename T>
        std::size_t JDManagerObjectManager::getObjectCount() const
        {
            size_t c = 0;
            for (auto& o : m_objs)
            {
                if (dynamic_cast<T*>(o))
                    ++c;
            }
            return c;
        }

        template<typename T>
        T* JDManagerObjectManager::getObject(const JDObjectIDptr& id)
        {
            const JDObject & obj = getObject(id);
            T* casted = dynamic_cast<T*>(obj);
            return casted;
        }

        template<typename T>
        T* JDManagerObjectManager::getObject(const JDObjectID::IDType& id)
        {
            const JDObject& obj = getObject(id);
            T* casted = dynamic_cast<T*>(obj);
            return casted;
        }

        template<typename T>
        std::vector<T*> JDManagerObjectManager::getObjects() const
        {
            std::vector<T*> list;
            list.reserve(m_objs.size());
            for (auto& p : m_objs)
            {
                T* obj = dynamic_cast<T*>(p);
                if (obj)
                    list.push_back(obj);
            }
            return list;
        }
    }
}