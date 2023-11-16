#include "object/JDObjectContainer.h"



namespace JsonDatabase
{

    JDObjectInterface* JDObjectContainer::operator[](const JDObjectID &id)
    {
		auto it = m_objectMap.find(id.get());
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
    bool JDObjectContainer::operator[](JDObjectInterface *obj)
    {
        auto it = m_objectPtrMap.find(obj);
        if (it != m_objectPtrMap.end())
        {
            return it->second;
        }
        return false;
    }

    void JDObjectContainer::reserve(size_t size)
    {
		m_objectVector.reserve(size);
        m_objectPtrMap.reserve(size);
        m_objectMap.reserve(size);
    }

    bool JDObjectContainer::addObject(JDObjectInterface* obj)
    {
        if (!obj)
            return false;
        const JDObjectID &id = obj->getObjectID();
        auto it = m_objectMap.find(id.get());
        if (exists(obj))
            return false;
        m_objectVector.emplace_back(obj);
        m_objectMap[id.get()] = obj;
        m_objectPtrMap[obj] = obj;
        return true;
    }
    bool JDObjectContainer::addObject(const std::vector<JDObjectInterface*>& objs)
    {
        bool success = true;
        m_objectVector.reserve(m_objectVector.size() + objs.size());
        //m_objectVector.insert(m_objectVector.end(), objs.begin(), objs.end());
        for (auto it = objs.begin(); it != objs.end(); ++it)
        {
            if (!*it)
            {
                success = false;
                continue;
            }
               
			auto it2 = m_objectPtrMap.find(*it);
			if (it2 != m_objectPtrMap.end())
			{
				success = false;
				continue;
			}
			const JDObjectID &id = (*it)->getObjectID();
			m_objectMap[id.get()] = *it;
            m_objectVector.emplace_back(*it);
            m_objectPtrMap[*it] = *it;
		}
        return success;
    }
    JDObjectInterface* JDObjectContainer::replaceObject(JDObjectInterface* replacement)
    {
        if (!replacement)
            return nullptr;
        const JDObjectID &id = replacement->getObjectID();
		auto it = m_objectMap.find(id.get());
		if (it != m_objectMap.end())
		{
			auto it2 = std::find(m_objectVector.begin(), m_objectVector.end(), it->second);
			*it2 = replacement;
            JDObjectInterface *old = it->second;
			it->second = replacement;
            m_objectPtrMap.erase(old);
            m_objectPtrMap[replacement] = replacement;
			return old;
		}
		return nullptr;
    }
    bool JDObjectContainer::removeObject(const JDObjectID &id)
    {
        auto it = m_objectMap.find(id.get());
        if (it != m_objectMap.end()) 
        {
            auto it2 = std::find(m_objectVector.begin(), m_objectVector.end(), it->second);

            m_objectVector.erase(it2);
            m_objectMap.erase(it);
            m_objectPtrMap.erase(it->second);
            return true;
        }
        return false;
    }
    bool JDObjectContainer::removeObject(JDObjectInterface *obj)
    {
        if(!obj)
			return false;
        
        auto it = m_objectPtrMap.find(obj);
        if (it == m_objectPtrMap.end())
            return false;

        const JDObjectID &id = obj->getObjectID();
        auto it2 = m_objectMap.find(id.get());
        m_objectMap.erase(it2);

        auto it3 = std::find(m_objectVector.begin(), m_objectVector.end(), obj);
        m_objectVector.erase(it3);

        return true;
    }
    bool JDObjectContainer::removeObject(const std::vector<JDObjectInterface*>& objs)
    {
        if (!objs.size() == 0)
            return true;

        for (auto& obj : objs) {
            auto it = m_objectPtrMap.find(obj);
            if (it == m_objectPtrMap.end())
                continue;

            const JDObjectID &id = obj->getObjectID();
            auto it2 = m_objectMap.find(id.get());
            m_objectMap.erase(it2);

            auto it3 = std::find(m_objectVector.begin(), m_objectVector.end(), obj);
            m_objectVector.erase(it3);
        }
        return true;
    }

    JDObjectInterface* JDObjectContainer::getObjectByID(const JDObjectID &id) 
    {
        auto it = m_objectMap.find(id.get());
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
    const std::unordered_map<JDObjectID::IDType, JDObjectInterface*>& JDObjectContainer::getAllObjectsIDMap() const
    {
        return m_objectMap;
    }
    const std::unordered_map<JDObjectInterface*, JDObjectInterface*>& JDObjectContainer::getAllObjectsPtrMap() const
    {
        return m_objectPtrMap;
    }

    bool JDObjectContainer::exists(const JDObjectID &id) const
    {
        
        auto it = m_objectMap.find(id.get());
        if (it != m_objectMap.end())
        {
            return true;
        }
        return false;
    }
    bool JDObjectContainer::exists(JDObjectInterface* obj) const
    {
        if (!obj)
			return false;
        return m_objectPtrMap.find(obj) != m_objectPtrMap.end();
        /*auto it = std::find(m_objectVector.begin(), m_objectVector.end(), obj);
        if(it != m_objectVector.end())
		{
			return true;
        }
        return false;*/
    }

    size_t JDObjectContainer::size() const
    {
		return m_objectVector.size();
    }
    void JDObjectContainer::clear()
    {
        m_objectVector.clear();
		m_objectMap.clear();
        m_objectPtrMap.clear();
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