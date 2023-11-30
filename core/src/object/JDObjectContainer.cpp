#include "object/JDObjectContainer.h"
#include "object/JDObjectInterface.h"


namespace JsonDatabase
{
    namespace Internal
    {
        JDObjectManager* JDObjectContainer::operator[](const JDObjectIDptr& id)
        {
            auto it = m_objectMap.find(id->get());
            if (it != m_objectMap.end())
            {
                return it->second;
            }
            return nullptr;
        }
        JDObjectManager* JDObjectContainer::operator[](size_t index)
        {

            if (m_objectVector.size() > index)
                return m_objectVector[index];
            return nullptr;
        }
        size_t JDObjectContainer::operator[](JDObject& obj)
        {
            for (size_t i = 0; i < m_objectVector.size(); ++i)
            {
                if (m_objectVector[i]->getObject().get() == obj.get())
                    return i;
            }
        }

        void JDObjectContainer::reserve(size_t size)
        {
            m_objectVector.reserve(size);
            m_objectPtrMap.reserve(size);
            m_objectMap.reserve(size);
        }

        bool JDObjectContainer::addObject(JDObjectManager* obj)
        {
            if (!obj)
                return false;
            const JDObjectIDptr id = obj->getID();
            auto it = m_objectMap.find(id->get());
            if (it != m_objectMap.end())
            {
                if (it->second == obj)
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
            m_objectPtrMap[obj->getObject().get()] = obj;
            return true;
        }
        bool JDObjectContainer::addObject(const std::vector<JDObjectManager*>& objs)
        {
            bool success = true;
            m_objectVector.reserve(m_objectVector.size() + objs.size());
            m_objectMap.reserve(m_objectMap.size() + objs.size());
            m_objectPtrMap.reserve(m_objectPtrMap.size() + objs.size());

            for (auto it = objs.begin(); it != objs.end(); ++it)
            {
                JDObjectManager* obj = *it;
                if (!obj)
                {
                    success = false;
                    continue;
                }

                JDObjectIDptr id = obj->getID();
                auto it2 = m_objectMap.find(id->get());
                if (it2 != m_objectMap.end())
                {
                    success = false;
                    // Object already added
                    if (it2->second == obj)
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
                m_objectPtrMap[obj->getObject().get()] = obj;
            }
            return success;
        }
        /*JDObject JDObjectContainer::replaceObject(const JDObject& replacement)
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
        }*/
        JDObjectManager* JDObjectContainer::getAndRemoveObject(const JDObjectIDptr& id)
        {
            auto it = m_objectMap.find(id->get());
            if (it != m_objectMap.end())
            {
                JDObjectManager *manager = it->second;
                auto it2 = std::find(m_objectVector.begin(), m_objectVector.end(), it->second);

                m_objectVector.erase(it2);
                m_objectMap.erase(it);
                m_objectPtrMap.erase(it->second->getObject().get());
                return manager;
            }
            return nullptr;
        }
        bool JDObjectContainer::removeObject(const JDObjectIDptr& id)
        {
            auto it = m_objectMap.find(id->get());
            if (it != m_objectMap.end())
            {
                auto it2 = std::find(m_objectVector.begin(), m_objectVector.end(), it->second);

                m_objectVector.erase(it2);
                m_objectMap.erase(it);
                m_objectPtrMap.erase(it->second->getObject().get());
                return true;
            }
            return false;
        }
        bool JDObjectContainer::removeObject(JDObjectManager *obj)
        {
            return removeObject(obj->getID());
        }
        bool JDObjectContainer::removeObject(const JDObject& obj)
        {
            return removeObject(obj->getObjectID());
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

        JDObjectManager* JDObjectContainer::getObjectByID(const JDObjectIDptr& id)
        {
            auto it = m_objectMap.find(id->get());
            if (it != m_objectMap.end())
            {
                return it->second;
            }
            return nullptr;
        }
        JDObjectManager* JDObjectContainer::getObjectByID(const JDObjectID::IDType& id)
        {
            auto it = m_objectMap.find(id);
            if (it != m_objectMap.end())
            {
                return it->second;
            }
            return nullptr;
        }
        JDObjectManager* JDObjectContainer::getObjectByPtr(const JDObjectInterface* obj)
        {
            auto it = m_objectPtrMap.find(obj);
            if (it != m_objectPtrMap.end())
            {
                return it->second;
            }
            return nullptr;
        }

        std::vector<JDObjectManager*> JDObjectContainer::getAllObjects() const
        {
            return m_objectVector;
        }
        std::unordered_map<JDObjectID::IDType, JDObjectManager*> JDObjectContainer::getAllObjectsIDMap() const
        {
            return m_objectMap;
        }
        std::unordered_map<const JDObjectInterface*, JDObjectManager*> JDObjectContainer::getAllObjectsPtrMap() const
        {
            return m_objectPtrMap;
        }

        bool JDObjectContainer::exists(const JDObjectIDptr& id) const
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
        }
        bool JDObjectContainer::exists(JDObjectManager* obj) const
        {
            return exists(obj->getObject());
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
}