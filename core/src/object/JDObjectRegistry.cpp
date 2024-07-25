#include "object/JDObjectRegistry.h"
#include "object/JDObjectInterface.h"

namespace JsonDatabase
{
    namespace Internal
    {
        JDObjectRegistry::JDObjectRegistry()
            : m_logger("JDObjectRegistry")
        {

        }
        JDObjectRegistry& JDObjectRegistry::getInstance() {
            static JDObjectRegistry instance;
            return instance;
        }
        Log::Logger::ContextLogger& JDObjectRegistry::getLogger()
        {
			return JDObjectRegistry::getInstance().m_logger;
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
                    instance.m_logger.logError("A object of type \"JDObjectInterface\" returns an empty string in the function \"className()\"\n"
                        "Did you forget to use the macro \"JD_OBJECT(derivedType)\" in the derived JDObjectInterface type header and\n"
                        "the macro \"JD_OBJECT_IMPL(derivedType)\" in the derived JDObjectInterface type cpp file?\n"
                        "See JDObjectInterface.h for a example, how to create a JDObjectInterface derived object.");
                    return error;
                }
                if (instance.m_registry.find(className) != instance.m_registry.end())
                {
                    error = Error::typeAlreadyRegistered;
                    instance.m_logger.logError("Error::typeAlreadyRegistered");
                    return error;
                }

                instance.m_registry.insert(std::pair<std::string, JDObject>(className, obj));
            }
            else
                error = Error::objIsNullptr;
            return error;
        }


        const JDObject& JDObjectRegistry::getObjectDefinition(const JsonObject& json)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
            std::string className;
            bool match = json.contains(JDObjectInterface::s_tag_className);
			if (match)
            {
                className = json.at(JDObjectInterface::s_tag_className).get<std::string>();
            
                return getObjectDefinition(className);
            }
            static const JDObject nullObj = nullptr;
            return nullObj;
        }

        std::string JDObjectRegistry::getObjectTypeString(const JsonObject& json)
        {
            std::string className;
            className = json.at(JDObjectInterface::s_tag_className).get<std::string>();
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