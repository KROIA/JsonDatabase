#include "object/JDObjectContainer.h"



namespace JsonDatabase
{

    JDObject JDObjectContainer::operator[](const JDObjectIDptr &id)
    {
		auto it = m_objectMap.find(id->get());
		if (it != m_objectMap.end())
		{
			return it->second;
		}
		return nullptr;
    }
    JDObject JDObjectContainer::operator[](size_t index)
    {
        
        if(m_objectVector.size() > index)
            return m_objectVector[index];
        return nullptr;
    }
    size_t JDObjectContainer::operator[](JDObject &obj)
    {
        for(size_t i=0; i<m_objectVector.size(); ++i)
		{
			if(m_objectVector[i].get() == obj.get())
				return i;
		}
    }

    void JDObjectContainer::reserve(size_t size)
    {
		m_objectVector.reserve(size);
        m_objectPtrMap.reserve(size);
        m_objectMap.reserve(size);
    }

    bool JDObjectContainer::addObject(const JDObject& obj)
    {
        if (!obj.get())
            return false;
        const JDObjectIDptr id = obj->getObjectID();
        auto it = m_objectMap.find(id->get());
        if (it != m_objectMap.end())
        {
            if (it->second.get() == obj.get())
            {
                // Object already added
                return false;
            }
            else
            {
                // ID already taken for a different instance
                return false;
            }
        }
        m_objectVector.emplace_back(obj);
        m_objectMap[id->get()] = obj;
        m_objectPtrMap[obj.get()] = obj;
        return true;
    }
    bool JDObjectContainer::addObject(const std::vector<JDObject>& objs)
    {
        bool success = true;
        m_objectVector.reserve(m_objectVector.size() + objs.size());
        m_objectMap.reserve(m_objectMap.size() + objs.size());
        m_objectPtrMap.reserve(m_objectPtrMap.size() + objs.size());

        for (auto it = objs.begin(); it != objs.end(); ++it)
        {
            JDObject obj = *it;
            if (!obj.get())
            {
                success = false;
                continue;
            }
               
            JDObjectIDptr id = obj->getObjectID();
			auto it2 = m_objectMap.find(id->get());
			if (it2 != m_objectMap.end())
			{
				success = false;
                // Object already added
                if (it2->second.get() == obj.get())
                {
                    // Object already added
                }
                else
                {
                    // ID already taken for a different instance
                }
				continue;
			}
            m_objectVector.emplace_back(obj);
			m_objectMap[id->get()] = obj;
            m_objectPtrMap[obj.get()] = obj;
		}
        return success;
    }
    JDObject JDObjectContainer::replaceObject(const JDObject& replacement)
    {
        if (!replacement.get())
            return nullptr;
        JDObjectIDptr id = replacement->getObjectID();
		auto it = m_objectMap.find(id->get());
		if (it != m_objectMap.end())
		{
			auto it2 = std::find(m_objectVector.begin(), m_objectVector.end(), it->second);
			*it2 = replacement;
            JDObject& old = it->second;
			it->second = replacement;
            m_objectPtrMap.erase(old.get());
            m_objectPtrMap[replacement.get()] = replacement;
			return old;
		}
		return nullptr;
    }
    bool JDObjectContainer::removeObject(const JDObjectIDptr &id)
    {
        auto it = m_objectMap.find(id->get());
        if (it != m_objectMap.end()) 
        {
            auto it2 = std::find(m_objectVector.begin(), m_objectVector.end(), it->second);

            m_objectVector.erase(it2);
            m_objectMap.erase(it);
            m_objectPtrMap.erase(it->second.get());
            return true;
        }
        return false;
    }
    bool JDObjectContainer::removeObject(const JDObject& obj)
    {
        if(!obj.get())
			return false;
        
        auto it = m_objectPtrMap.find(obj.get());
        if (it == m_objectPtrMap.end())
            return false;

        m_objectPtrMap.erase(it);

        JDObjectIDptr id = obj->getObjectID();
        auto it2 = m_objectMap.find(id->get());
        m_objectMap.erase(it2);

        auto it3 = std::find(m_objectVector.begin(), m_objectVector.end(), obj);
        m_objectVector.erase(it3);

        

        return true;
    }
    bool JDObjectContainer::removeObject(const std::vector<JDObject>& objs)
    {
        if (objs.size() == 0)
            return true;

        for (auto& obj : objs) {
            auto it = m_objectPtrMap.find(obj.get());
            if (it == m_objectPtrMap.end())
                continue;

            JDObjectIDptr id = obj->getObjectID();
            auto it2 = m_objectMap.find(id->get());
            m_objectMap.erase(it2);

            auto it3 = std::find(m_objectVector.begin(), m_objectVector.end(), obj);
            m_objectVector.erase(it3);
        }
        return true;
    }

    JDObject JDObjectContainer::getObjectByID(const JDObjectIDptr &id) 
    {
        auto it = m_objectMap.find(id->get());
        if (it != m_objectMap.end()) 
        {
            return it->second;
        }
        return nullptr;
    }
    JDObject JDObjectContainer::getObjectByID(const JDObjectID::IDType& id)
    {
        auto it = m_objectMap.find(id);
        if (it != m_objectMap.end())
        {
            return it->second;
        }
        return nullptr;
    }
    JDObject JDObjectContainer::getObjectByPtr(const JDObjectInterface* obj)
    {
        auto it = m_objectPtrMap.find(obj);
        if (it != m_objectPtrMap.end())
        {
			return it->second;
		}
		return nullptr;
    }

    std::vector<JDObject> JDObjectContainer::getAllObjects() const 
    {
        return m_objectVector;
    }
    std::unordered_map<JDObjectID::IDType, JDObject> JDObjectContainer::getAllObjectsIDMap() const
    {
        return m_objectMap;
    }
    std::unordered_map<const JDObjectInterface*, JDObject> JDObjectContainer::getAllObjectsPtrMap() const
    {
        return m_objectPtrMap;
    }

    bool JDObjectContainer::exists(const JDObjectIDptr &id) const
    {
        
        auto it = m_objectMap.find(id->get());
        if (it != m_objectMap.end())
        {
            return true;
        }
        return false;
    }
    bool JDObjectContainer::exists(const JDObject& obj) const
    {
        if (!obj.get())
			return false;
        return m_objectPtrMap.find(obj.get()) != m_objectPtrMap.end();
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