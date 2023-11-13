#include "object/JDObjectRegistry.h"
#include "object/JDObjectInterface.h"

namespace JsonDatabase
{
    JDObjectRegistry::JDObjectRegistry()
    {

    }
    JDObjectRegistry& JDObjectRegistry::getInstance() {
        static JDObjectRegistry instance;
        return instance;
    }

    JDObjectRegistry::Error JDObjectRegistry::registerType(JDObjectInterface* obj) {
        Error error = Error::none;
        JDObjectRegistry& instance = getInstance();
        
        if (obj)
        {
            const std::string& className = obj->className();
            if (className.size() == 0)
            {
                error = Error::emptyClassName;
                JD_CONSOLE_FUNCTION("\nError: A object of type \"JDObjectInterface\" returns an empty string in the function \"className()\"\n"
                                    "Did you forget to use the macro \"JD_OBJECT(derivedType)\" in the derived JDObjectInterface type header and\n"
                                    "the macro \"JD_OBJECT_IMPL(derivedType)\" in the derived JDObjectInterface type cpp file?\n"
                                    "See JDObjectInterface.h for a example, how to create a JDObjectInterface derived object.\n");
                return error;
            }
            if (instance.m_registry.find(className) != instance.m_registry.end())
            {
                error = Error::typeAlreadyRegistered;
                JD_CONSOLE_FUNCTION("Error::typeAlreadyRegistered");
                return error;
            }

            instance.m_registry.insert(std::pair<std::string, JDObjectInterface*>(className, obj));
        }
        else
            error = Error::objIsNullptr;
        return error;
    }

    JDObjectInterface* JDObjectRegistry::getObjectDefinition(const QJsonObject& json)
    {
        std::string className;
        if (JDSerializable::getJsonValue(json, className, JDObjectInterface::s_tag_className))
        {
            return getObjectDefinition(className);
        }
        return nullptr;
    }
    JDObjectInterface* JDObjectRegistry::getObjectDefinition(const std::string& className)
    {
        const std::map<std::string, JDObjectInterface*>& registry = getRegisteredTypes();
        auto it = registry.find(className);
        if (it == registry.end())
            return nullptr;
        return it->second;
    }

    const std::map<std::string, JDObjectInterface*>& JDObjectRegistry::getRegisteredTypes() 
    {
        return getInstance().m_registry;
    }
}