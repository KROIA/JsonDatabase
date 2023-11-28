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

            
            bool addObject(const JDObject & obj);
            bool addObject(const std::vector<const JDObject &>& objList);

            const JDObject & replaceObject(const JDObject & obj);
            std::vector<const JDObject &> replaceObjects(const std::vector<const JDObject &>& objList);
           
            bool removeObject(const JDObject & obj);
            bool removeObjects(const std::vector<const JDObject &>& objList);
            template<typename T>
            bool removeObjects();
            template<typename T>
            
            bool deleteObjects();
            
            template<typename T>
            size_t getObjectCount() const;
            size_t getObjectCount() const;
            
            bool exists(const JDObject & obj) const;
            bool exists(const JDObjectIDptr&id) const;

            template<typename T>
            T* getObject(const JDObjectIDptr&id);
            const JDObject & getObject(const JDObjectIDptr&id);
            template<typename T>
            std::vector<T*> getObjects() const;
            const std::vector<const JDObject &>& getObjects() const;

            void clearObjects();
        protected:

            bool addObject_internal(const JDObject & obj);
            bool addObject_internal(const std::vector<const JDObject &>& objs);
            const JDObject & replaceObject_internal(const JDObject & obj);
            void replaceObject_internal(const std::vector<const JDObject &>& objs);
            bool removeObject_internal(const JDObject & obj);
            bool removeObject_internal(const std::vector<const JDObject &>& objs);
            bool exists_internal(const JDObject & obj) const;
            bool exists_internal(const JDObjectIDptr&id) const;
            const JDObject & getObject_internal(const JDObjectIDptr&id);
            const std::vector<const JDObject &>& getObjects_internal() const;
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
        T* JDManagerObjectManager::getObject(const JDObjectIDptr&id)
        {
            const JDObject & obj = getObject(id);
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