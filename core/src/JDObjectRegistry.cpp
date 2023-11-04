#include "JDObjectRegistry.h"

namespace JsonDatabase
{
    JDObjectRegistry::JDObjectRegistry()
    {

    }
    JDObjectRegistry& JDObjectRegistry::getInstance() {
        static JDObjectRegistry instance;
        return instance;
    }

    int JDObjectRegistry::registerType(JDObjectInterface* obj) {
        int error = 0;
        JDObjectRegistry& instance = getInstance();
        std::string className;
        if (obj)
        {
            className = obj->className();
            if (className.size() == 0)
                error = 1;
            if (instance.m_registry.find(className) != instance.m_registry.end())
                error = 2;
        }
        else
            error = 3;
        if (error)
        {
            return error;
        }
        instance.m_registry.insert(std::pair<std::string, JDObjectInterface*>(className, obj));
        return error;
    }

    const std::map<std::string, JDObjectInterface*>& JDObjectRegistry::getRegisteredTypes() 
    {
        return getInstance().m_registry;
    }
}