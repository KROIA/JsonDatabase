#pragma once

#include "JD_base.h"
#include "JDObjectInterface.h"
#include <map>

namespace JsonDatabase
{
    namespace Internal
    {
        class JSONDATABASE_EXPORT JDObjectRegistry
        {
            JDObjectRegistry();
        public:
            enum Error
            {
                none,
                emptyClassName,
                typeAlreadyRegistered,
                objIsNullptr
            };


            static JDObjectRegistry& getInstance();


            static Error registerType(const JDObject& obj);
            static const std::map<std::string, JDObject>& getRegisteredTypes();
#if JD_ACTIVE_JSON == JD_JSON_QT
            static const JDObject& getObjectDefinition(const QJsonObject& json);
            static std::string getObjectTypeString(const QJsonObject& json);
#elif JD_ACTIVE_JSON == JD_JSON_GLAZE || JD_ACTIVE_JSON == JD_JSON_INTERNAL
            static const JDObject& getObjectDefinition(const JsonObject& json);
            static std::string getObjectTypeString(const JsonObject& json);
#endif
            static const JDObject& getObjectDefinition(const std::string& className);

        private:
            std::map<std::string, JDObject> m_registry;
        };
    }
}