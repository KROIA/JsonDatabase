#include "JDObjectContainer.h"



namespace JsonDatabase
{

    JDObjectInterface* JDObjectContainer::operator[](const std::string& id)
    {
		auto it = m_objectMap.find(id);
		if (it != m_objectMap.end())
		{
			return it->second;
		}
		return nullptr;
    }
    JDObjectInterface* JDObjectContainer::operator[](size_t index)
    {
        if(m_objectVector.size() > index)
            return m_objectVector[index];
        return nullptr;
    }

    void JDObjectContainer::reserve(size_t size)
    {
		m_objectVector.reserve(size);
    }

    void JDObjectContainer::addObject(JDObjectInterface* obj) 
    {
        std::string id = obj->getObjectID();
        m_objectVector.push_back(obj);
        m_objectMap[id] = obj;
    }
    JDObjectInterface* JDObjectContainer::replaceObject(JDObjectInterface* replacement)
    {
        std::string id = replacement->getObjectID();
		auto it = m_objectMap.find(id);
		if (it != m_objectMap.end())
		{
			auto it2 = std::find(m_objectVector.begin(), m_objectVector.end(), it->second);
			*it2 = replacement;
            JDObjectInterface *old = it->second;
			it->second = replacement;
			return old;
		}
		return nullptr;
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
    bool JDObjectContainer::exists(const std::string& id) const
    {
        auto it = m_objectMap.find(id);
        if (it != m_objectMap.end())
        {
            return true;
        }
        return false;
    }
    bool JDObjectContainer::exists(JDObjectInterface* obj) const
    {
        auto it = std::find(m_objectVector.begin(), m_objectVector.end(), obj);
        if(it != m_objectVector.end())
		{
			return true;
        }
        return false;
    }

    size_t JDObjectContainer::size() const
    {
		return m_objectVector.size();
    }
    void JDObjectContainer::clear()
    {
        m_objectVector.clear();
		m_objectMap.clear();
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