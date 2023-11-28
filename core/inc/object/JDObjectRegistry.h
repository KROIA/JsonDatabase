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
#ifdef JD_USE_QJSON
            static const JDObject& getObjectDefinition(const QJsonObject& json);
#else
            static const JDObject& getObjectDefinition(const JsonValue& json);
#endif
            static const JDObject& getObjectDefinition(const std::string& className);

        private:
            std::map<std::string, JDObject> m_registry;
        };
    }
}