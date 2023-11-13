#include "manager/JDManagerObjectManager.h"
#include "utilities/JDUniqueMutexLock.h"

namespace JsonDatabase
{
    namespace Internal
    {
        JDManagerObjectManager::JDManagerObjectManager(std::mutex& mtx)
            : m_mutex(mtx)
        {

        }
        JDManagerObjectManager::~JDManagerObjectManager()
        {

        }
        void JDManagerObjectManager::setup()
        {

        }
        
        
        bool JDManagerObjectManager::addObject(JDObjectInterface* obj)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
            JDM_UNIQUE_LOCK_P;
            return addObject_internal(obj);
        }
        bool JDManagerObjectManager::addObject(const std::vector<JDObjectInterface*>& objList)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
            JDM_UNIQUE_LOCK_P;
            bool success = true;
            for (size_t i = 0; i < objList.size(); ++i)
                success &= addObject_internal(objList[i]);
            return success;
        }
        
        JDObjectInterface* JDManagerObjectManager::replaceObject(JDObjectInterface* obj)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
            JDM_UNIQUE_LOCK_P;
            return replaceObject_internal(obj);
        }
        std::vector<JDObjectInterface*> JDManagerObjectManager::replaceObjects(const std::vector<JDObjectInterface*>& objList)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
            JDM_UNIQUE_LOCK_P;
            std::vector<JDObjectInterface*> replacedObjs;
            replacedObjs.reserve(objList.size());
            for (size_t i = 0; i < objList.size(); ++i)
            {
                replacedObjs.push_back(replaceObject_internal(objList[i]));
            }
            return replacedObjs;
        }
        
        bool JDManagerObjectManager::removeObject(JDObjectInterface* obj)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
            JDM_UNIQUE_LOCK_P;
            return removeObject_internal(obj);
        }
        
        bool JDManagerObjectManager::removeObjects(const std::vector<JDObjectInterface*>& objList)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
            JDM_UNIQUE_LOCK_P;
            bool success = true;
            for (size_t i = 0; i < objList.size(); ++i)
                success &= removeObject_internal(objList[i]);
            return success;
        }
        std::size_t JDManagerObjectManager::getObjectCount() const
        {
            JDM_UNIQUE_LOCK_P;
            return m_objs.size();
        }
        bool JDManagerObjectManager::exists(JDObjectInterface* obj) const
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
            JDM_UNIQUE_LOCK_P;
            return exists_internal(obj);
        }
        bool JDManagerObjectManager::exists(const std::string& id) const
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
            JDM_UNIQUE_LOCK_P;
            return exists_internal(id);
        }
        JDObjectInterface* JDManagerObjectManager::getObject(const std::string& objID)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
            JDM_UNIQUE_LOCK_P;
            return getObject_internal(objID);
        }
        const std::vector<JDObjectInterface*>& JDManagerObjectManager::getObjects() const
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
            JDM_UNIQUE_LOCK_P;
            return getObjects_internal();
        }

        void JDManagerObjectManager::clearObjects()
        {
            JDM_UNIQUE_LOCK_P;
            clearObjects_internal();
        }

        /*
          -----------------------------------------------------------------------------------------------
          ------------------ I N T E R N A L ------------------------------------------------------------
          -----------------------------------------------------------------------------------------------
        */

        bool JDManagerObjectManager::addObject_internal(JDObjectInterface* obj)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            if (!obj) return false;
            if (exists_internal(obj->getObjectID())) return false;
            if (exists_internal(obj)) return false;
            m_objs.addObject(obj);
            return true;
        }
        JDObjectInterface* JDManagerObjectManager::replaceObject_internal(JDObjectInterface* obj)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            return m_objs.replaceObject(obj);
        }
        bool JDManagerObjectManager::removeObject_internal(JDObjectInterface* obj)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            if (!obj) return false;
            if (!exists_internal(obj) && !exists_internal(obj->getObjectID())) return false;
            m_objs.removeObject(obj);
            return true;
        }
        bool JDManagerObjectManager::exists_internal(JDObjectInterface* obj) const
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            if (!obj) return false;
            return m_objs.exists(obj);
        }
        bool JDManagerObjectManager::exists_internal(const std::string& id) const
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            return m_objs.exists(id);
        }
        JDObjectInterface* JDManagerObjectManager::getObject_internal(const std::string& objID)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            if (objID.size() == 0)
                return nullptr;
            return m_objs[objID];
        }
        const std::vector<JDObjectInterface*>& JDManagerObjectManager::getObjects_internal() const
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            return m_objs.getAllObjects();
        }
        void JDManagerObjectManager::clearObjects_internal()
        {
            m_objs.clear();
        }

    }
}