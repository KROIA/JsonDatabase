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
            friend class JDManager;
			JDManagerObjectManager(std::mutex &mtx);
            virtual ~JDManagerObjectManager();
            void setup();
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
            bool exists(const std::string& id) const;

            template<typename T>
            T* getObject(const std::string& objID);
            JDObjectInterface* getObject(const std::string& objID);
            template<typename T>
            std::vector<T*> getObjects() const;
            const std::vector<JDObjectInterface*>& getObjects() const;

            void clearObjects();
        protected:

            bool addObject_internal(JDObjectInterface* obj);
            JDObjectInterface* replaceObject_internal(JDObjectInterface* obj);
            bool removeObject_internal(JDObjectInterface* obj);
            bool exists_internal(JDObjectInterface* obj) const;
            bool exists_internal(const std::string& id) const;
            JDObjectInterface* getObject_internal(const std::string& objID);
            const std::vector<JDObjectInterface*>& getObjects_internal() const;
            void clearObjects_internal();


        private:
            std::mutex &m_mutex;
            JDObjectContainer m_objs;
        };



        template<typename T>
        bool JDManagerObjectManager::removeObjects()
        {
            JD_GENERAL_PROFILING_FUNCTION(COLOR_STAGE_1)

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
            JD_GENERAL_PROFILING_FUNCTION(COLOR_STAGE_1)


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
        T* JDManagerObjectManager::getObject(const std::string& objID)
        {
            JDObjectInterface* obj = getObject(objID);
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