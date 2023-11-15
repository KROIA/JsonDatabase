#pragma once

#include "JD_base.h"
#include "JDObjectInterface.h"
#include <vector>
#include <unordered_map>

namespace JsonDatabase
{
    class JSONDATABASE_EXPORT JDObjectContainer
    {
    public:
        using iterator = std::vector<JDObjectInterface*>::iterator;
        using const_iterator = std::vector<JDObjectInterface*>::const_iterator;

        JDObjectInterface* operator[](const std::string& id);
	    JDObjectInterface* operator[](size_t index);
	    bool operator[](JDObjectInterface *obj); // returns true if the object exists in the container

        void reserve(size_t size);

        bool addObject(JDObjectInterface* obj);
        bool addObject(const std::vector<JDObjectInterface*>& objs);
        JDObjectInterface* replaceObject(JDObjectInterface* replacement); // Returns the old object
        bool removeObject(const std::string& id);
        bool removeObject(JDObjectInterface *obj);
        bool removeObject(const std::vector<JDObjectInterface*>& objs);

        JDObjectInterface* getObjectByID(const std::string& id);
        const std::vector<JDObjectInterface*>& getAllObjects() const;
        const std::unordered_map<std::string, JDObjectInterface*>& getAllObjectsIDMap() const;
        const std::unordered_map<JDObjectInterface*, JDObjectInterface*>& getAllObjectsPtrMap() const;

        bool exists(const std::string& id) const;
        bool exists(JDObjectInterface* obj) const;

        size_t size() const;
        void clear();

        iterator begin();
        iterator end();

        const_iterator begin() const;
        const_iterator end() const;
    private:
        std::vector<JDObjectInterface*> m_objectVector;
        std::unordered_map<std::string, JDObjectInterface*> m_objectMap;
        std::unordered_map<JDObjectInterface*, JDObjectInterface*> m_objectPtrMap;
    };
}