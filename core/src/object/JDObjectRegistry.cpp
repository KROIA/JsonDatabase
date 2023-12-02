#include "object/JDObjectRegistry.h"
#include "object/JDObjectInterface.h"

namespace JsonDatabase
{
    namespace Internal
    {
        JDObjectRegistry::JDObjectRegistry()
        {

        }
        JDObjectRegistry& JDObjectRegistry::getInstance() {
            static JDObjectRegistry instance;
            return instance;
        }

        JDObjectRegistry::Error JDObjectRegistry::registerType(const JDObject& obj) {
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

                instance.m_registry.insert(std::pair<std::string, JDObject>(className, obj));
            }
            else
                error = Error::objIsNullptr;
            return error;
        }

#ifdef JD_USE_QJSON
        const JDObject& JDObjectRegistry::getObjectDefinition(const QJsonObject& json)
#else
        const JDObject& JDObjectRegistry::getObjectDefinition(const JsonValue& json)
#endif
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
            std::string className;
#ifdef JD_USE_QJSON
            if (Utilities::JDSerializable::getJsonValue(json, className, JDObjectInterface::s_tag_className))
#else
            if (json.extractString(className, JDObjectInterface::s_tag_className))
#endif
            {
                return getObjectDefinition(className);
            }
            static const JDObject nullObj = nullptr;
            return nullObj;
        }

#ifdef JD_USE_QJSON
        std::string JDObjectRegistry::getObjectTypeString(const QJsonObject& json)
#else
        std::string JDObjectRegistry::getObjectTypeString(const JsonValue& json)
#endif
        {
            std::string className;
#ifdef JD_USE_QJSON
            Utilities::JDSerializable::getJsonValue(json, className, JDObjectInterface::s_tag_className);
#else
            json.extractString(className, JDObjectInterface::s_tag_className);
#endif
            return className;
        }

        const JDObject& JDObjectRegistry::getObjectDefinition(const std::string& className)
        {
            const std::map<std::string, JDObject>& registry = getRegisteredTypes();
            auto it = registry.find(className);
            if (it == registry.end())
            {
                static const JDObject nullObj = nullptr;
                return nullObj;
            }
            return it->second;
        }

        const std::map<std::string, JDObject>& JDObjectRegistry::getRegisteredTypes()
        {
            return getInstance().m_registry;
        }
    }
}