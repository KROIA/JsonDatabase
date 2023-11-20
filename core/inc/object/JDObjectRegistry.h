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


            static Error registerType(JDObjectInterface* obj);
            static const std::map<std::string, JDObjectInterface*>& getRegisteredTypes();
#ifdef JD_USE_QJSON
            static JDObjectInterface* getObjectDefinition(const QJsonObject& json);
#else
            static JDObjectInterface* getObjectDefinition(const JsonValue& json);
#endif
            static JDObjectInterface* getObjectDefinition(const std::string& className);

        private:
            std::map<std::string, JDObjectInterface*> m_registry;
        };
    }
}