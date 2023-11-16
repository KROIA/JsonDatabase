#pragma once

#include "JD_base.h"
#include "JDDeclaration.h"
#include "object/JDObjectContainer.h"

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
			JDManagerObjectManager(std::mutex &mtx);
            virtual ~JDManagerObjectManager();
            bool setup();
        public:

            
            bool addObject(JDObjectInterface* obj);
            bool addObject(const std::vector<JDObjectInterface*>& objList);

            JDObjectInterface* replaceObject(JDObjectInterface* obj);
            std::vector<JDObjectInterface*> replaceObjects(const std::vector<JDObjectInterface*>& objList);
           
            bool removeObject(JDObjectInterface* obj);
            bool removeObjects(const std::vector<JDObjectInterface*>& objList);
            template<typename T>
            bool removeObjects();
            template<typename T>
            
            bool deleteObjects();
            
            template<typename T>
            size_t getObjectCount() const;
            size_t getObjectCount() const;
            
            bool exists(JDObjectInterface* obj) const;
            bool exists(const JDObjectID &id) const;

            template<typename T>
            T* getObject(const JDObjectID &id);
            JDObjectInterface* getObject(const JDObjectID &id);
            template<typename T>
            std::vector<T*> getObjects() const;
            const std::vector<JDObjectInterface*>& getObjects() const;

            void clearObjects();
        protected:

            bool addObject_internal(JDObjectInterface* obj);
            bool addObject_internal(const std::vector<JDObjectInterface*>& objs);
            JDObjectInterface* replaceObject_internal(JDObjectInterface* obj);
            void replaceObject_internal(const std::vector<JDObjectInterface*>& objs);
            bool removeObject_internal(JDObjectInterface* obj);
            bool removeObject_internal(const std::vector<JDObjectInterface*>& objs);
            bool exists_internal(JDObjectInterface* obj) const;
            bool exists_internal(const JDObjectID &id) const;
            JDObjectInterface* getObject_internal(const JDObjectID &id);
            const std::vector<JDObjectInterface*>& getObjects_internal() const;
            void clearObjects_internal();

            void update();
        private:
            std::mutex &m_mutex;

            mutable std::mutex m_objsMutex;
            JDObjectContainer m_objs;
        };



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
        T* JDManagerObjectManager::getObject(const JDObjectID &id)
        {
            JDObjectInterface* obj = getObject(id);
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