#include "JDObjectContainer.h"



namespace JsonDatabase
{

    void JDObjectContainer::addObject(JDObjectInterface* obj) 
    {
        std::string id = obj->getObjectID();
        m_objectVector.push_back(obj);
        m_objectMap[id] = obj;
    }
    void JDObjectContainer::removeObject(const std::string& id)
    {
        auto it = m_objectMap.find(id);
        if (it != m_objectMap.end()) 
        {
            auto it2 = std::find(m_objectVector.begin(), m_objectVector.end(), it->second);

            m_objectVector.erase(it2);
            m_objectMap.erase(it);
        }
    }
    void JDObjectContainer::removeObject(JDObjectInterface *obj)
    {
        if(!obj)
			return;
        std::string id = obj->getObjectID();
        auto it = m_objectMap.find(id);
        if (it != m_objectMap.end())
        {
            auto it2 = std::find(m_objectVector.begin(), m_objectVector.end(), obj);

            m_objectVector.erase(it2);
            m_objectMap.erase(it);
        }
    }

    JDObjectInterface* JDObjectContainer::getObjectByID(const std::string& id) 
    {
        auto it = m_objectMap.find(id);
        if (it != m_objectMap.end()) 
        {
            return it->second;
        }
        return nullptr;
    }

    const std::vector<JDObjectInterface*>& JDObjectContainer::getAllObjects() const 
    {
        return m_objectVector;
    }

    JDObjectContainer::iterator JDObjectContainer::begin() 
    {
        return m_objectVector.begin();
    }

    JDObjectContainer::iterator JDObjectContainer::end() 
    {
        return m_objectVector.end();
    }

    JDObjectContainer::const_iterator JDObjectContainer::begin() const 
    {
        return m_objectVector.begin();
    }

    JDObjectContainer::const_iterator JDObjectContainer::end() const 
    {
        return m_objectVector.end();
    }
}