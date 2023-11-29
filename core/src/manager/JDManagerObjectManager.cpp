#include "manager/JDManagerObjectManager.h"
#include "utilities/JDUniqueMutexLock.h"

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
            checkObjectIDAndFix_internal(obj);
            return addObject_internal(obj);
        }
        bool JDManagerObjectManager::addObject(const std::vector<JDObject>& objList)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
            JDM_UNIQUE_LOCK_P_M(m_objsMutex);
            bool success = true;
            for (size_t i = 0; i < objList.size(); ++i)
            {
                JDObject obj = objList[i];
                checkObjectIDAndFix_internal(obj);
                success &= addObject_internal(obj);
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
                checkObjectIDAndFix_internal(objList[i]);
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

        void JDManagerObjectManager::checkObjectIDAndFix_internal(const JDObject& obj)
        {
            if(!objectIDIsValid(obj))
                newObjectInstantiated_internal(obj);
        }

        void JDManagerObjectManager::newObjectInstantiated_internal(const JDObject& obj)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            JDObjectIDptr id = m_idDomain.getNewID();
            obj->setObjectID(id);
        }

        bool JDManagerObjectManager::addObject_internal(const JDObject & obj)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            obj->m_onDelete.connectSlot(this, &JDManagerObjectManager::onObjectGotDeleted);
            return m_objs.addObject(obj);
        }
        bool JDManagerObjectManager::addObject_internal(const std::vector<JDObject>& objs)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            for(auto obj : objs)
                obj->m_onDelete.connectSlot(this, &JDManagerObjectManager::onObjectGotDeleted);
            return m_objs.addObject(objs);
        }
        JDObject JDManagerObjectManager::replaceObject_internal(const JDObject & obj)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            obj->m_onDelete.connectSlot(this, &JDManagerObjectManager::onObjectGotDeleted);
            JDObject replacedObj = m_objs.replaceObject(obj);
            if(replacedObj.get())
                replacedObj->m_onDelete.disconnectSlot(this, &JDManagerObjectManager::onObjectGotDeleted);
            return replacedObj;
        }
        void JDManagerObjectManager::replaceObject_internal(const std::vector<JDObject>& objs)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            for (size_t i = 0; i < objs.size(); ++i)
            {
                objs[i]->m_onDelete.connectSlot(this, &JDManagerObjectManager::onObjectGotDeleted);
                JDObject replacedObj = m_objs.replaceObject(objs[i]);
                if (replacedObj.get())
                    replacedObj->m_onDelete.disconnectSlot(this, &JDManagerObjectManager::onObjectGotDeleted);
            }
        }
        bool JDManagerObjectManager::removeObject_internal(const JDObject & obj)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            obj->m_onDelete.disconnectSlot(this, &JDManagerObjectManager::onObjectGotDeleted);
            return m_objs.removeObject(obj);
        }
        bool JDManagerObjectManager::removeObject_internal(const std::vector<JDObject>& objs)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            for (auto obj : objs)
				obj->m_onDelete.disconnectSlot(this, &JDManagerObjectManager::onObjectGotDeleted);
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
            return m_objs.getObjectByID(id);
        }
        JDObject JDManagerObjectManager::getObject_internal(const JDObjectID::IDType& id)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            return m_objs.getObjectByID(id);
        }
        std::vector<JDObject> JDManagerObjectManager::getObjects_internal() const
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            return m_objs.getAllObjects();
        }
        void JDManagerObjectManager::clearObjects_internal()
        {
            m_objs.clear();
        }

        void JDManagerObjectManager::onObjectGotDeleted(const JDObjectInterface* obj)
        {
            JDObject sharedObj = m_objs.getObjectByPtr(obj);
            bool success = false;
            if(sharedObj.get())
                success = removeObject(sharedObj);
            sharedObj.reset();
        }

        void JDManagerObjectManager::update()
        {

        }

    }
}