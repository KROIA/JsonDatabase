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
        using iterator = std::vector<JDObject>::iterator;
        using const_iterator = std::vector<JDObject>::const_iterator;

        JDObject operator[](const JDObjectIDptr &id);
	    JDObject operator[](size_t index);
	    size_t operator[](JDObject &obj);

        void reserve(size_t size);

        bool addObject(const JDObject& obj);
        bool addObject(const std::vector<JDObject>& objs);
        JDObject replaceObject(const JDObject& replacement); // Returns the old object
        bool removeObject(const JDObjectIDptr &id);
        bool removeObject(const JDObject &obj);
        bool removeObject(const std::vector<JDObject>& objs);

        JDObject getObjectByID(const JDObjectIDptr &id);
        JDObject getObjectByID(const JDObjectID::IDType &id);
        JDObject getObjectByPtr(const JDObjectInterface *obj);
        std::vector<JDObject> getAllObjects() const;
        std::unordered_map<JDObjectID::IDType, JDObject> getAllObjectsIDMap() const;
        std::unordered_map<const JDObjectInterface*, JDObject> getAllObjectsPtrMap() const;

        bool exists(const JDObjectIDptr &id) const;
        bool exists(const JDObject& obj) const;

        size_t size() const;
        void clear();

        iterator begin();
        iterator end();

        const_iterator begin() const;
        const_iterator end() const;
    private:
        std::vector<JDObject> m_objectVector;
        std::unordered_map<JDObjectID::IDType, JDObject> m_objectMap;
        std::unordered_map<const JDObjectInterface*, JDObject> m_objectPtrMap;
    };
}