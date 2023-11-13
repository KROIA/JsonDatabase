#include "object/JDObjectContainer.h"



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
        const std::string &id = obj->getObjectID();
        m_objectVector.push_back(obj);
        m_objectMap[id] = obj;
    }
    void JDObjectContainer::addObject(const std::vector<JDObjectInterface*>& obj)
    {
        m_objectVector.reserve(m_objectVector.size() + obj.size());
        m_objectVector.insert(m_objectVector.end(), obj.begin(), obj.end());
        for (auto it = obj.begin(); it != obj.end(); ++it)
        {
			m_objectMap[(*it)->getObjectID()] = *it;
		}
    }
    JDObjectInterface* JDObjectContainer::replaceObject(JDObjectInterface* replacement)
    {
        const std::string &id = replacement->getObjectID();
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
        const std::string &id = obj->getObjectID();
        auto it = m_objectMap.find(id);
        if (it != m_objectMap.end())
        {
            
            m_objectMap.erase(it);
        }
        auto it2 = std::find(m_objectVector.begin(), m_objectVector.end(), obj);
        if(it2 != m_objectVector.end())
            m_objectVector.erase(it2);
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