#pragma once

#include "JD_base.h"
#include "JDObjectInterface.h"
#include <vector>
#include <map>

namespace JsonDatabase
{
    class JSONDATABASE_EXPORT JDObjectContainer
    {
    public:
        using iterator = std::vector<JDObjectInterface*>::iterator;
        using const_iterator = std::vector<JDObjectInterface*>::const_iterator;

        JDObjectInterface* operator[](const std::string& id);
	    JDObjectInterface* operator[](size_t index);

        void reserve(size_t size);

        void addObject(JDObjectInterface* obj);
        JDObjectInterface* replaceObject(JDObjectInterface* replacement); // Returns the old object
        void removeObject(const std::string& id);
        void removeObject(JDObjectInterface *obj);

        JDObjectInterface* getObjectByID(const std::string& id);
        const std::vector<JDObjectInterface*>& getAllObjects() const;

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
        std::map<std::string, JDObjectInterface*> m_objectMap;

    };
}