#pragma once

#include "JD_base.h"
#include "JDObjectManager.h"
#include "JDObjectID.h"
#include <vector>
#include <unordered_map>

namespace JsonDatabase
{
    namespace Internal
    {
        class JSONDATABASE_EXPORT JDObjectContainer
        {
        public:
            using iterator = std::vector<JDObjectManager*>::iterator;
            using const_iterator = std::vector<JDObjectManager*>::const_iterator;

            JDObjectManager* operator[](const JDObjectIDptr& id);
            JDObjectManager* operator[](size_t index);
            size_t operator[](JDObject& obj);

            void reserve(size_t size);

            bool addObject(JDObjectManager* obj);
            bool addObject(const std::vector<JDObjectManager*>& objs);
          //  JDObject replaceObject(const JDObject& replacement); // Returns the old object
            
            JDObjectManager* getAndRemoveObject(const JDObjectIDptr& id);
            bool removeObject(const JDObjectIDptr& id);
            bool removeObject(const JDObject& obj);
            bool removeObject(JDObjectManager* obj);
            bool removeObject(const std::vector<JDObject>& objs);
            bool removeObject(const std::vector<JDObjectManager*>& objs);

            JDObjectManager* getObjectByID(const JDObjectIDptr& id);
            JDObjectManager* getObjectByID(const JDObjectID::IDType& id);
            JDObjectManager* getObjectByPtr(JDObjectInterface* obj);
            const std::vector<JDObjectManager*> &getAllObjects() const;
            const std::unordered_map<JDObjectID::IDType, JDObjectManager*>& getAllObjectsIDMap() const;
            const std::unordered_map<JDObjectInterface*, JDObjectManager*>& getAllObjectsPtrMap() const;

            bool exists(const JDObjectIDptr& id) const;
            bool exists(const JDObject& obj) const;
            bool exists(JDObjectManager* obj) const;

            size_t size() const;
            void clear();

            iterator begin();
            iterator end();

            const_iterator begin() const;
            const_iterator end() const;
        private:
            std::vector<JDObjectManager*> m_objectVector;
            std::unordered_map<JDObjectID::IDType, JDObjectManager*> m_objectMap;
            std::unordered_map<JDObjectInterface*, JDObjectManager*> m_objectPtrMap;
        };
    }
}