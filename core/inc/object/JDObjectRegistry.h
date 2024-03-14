#pragma once

#include "JsonDatabase_base.h"
#include "JDObjectInterface.h"
#include <map>

namespace JsonDatabase
{
    namespace Internal
    {
        class JSON_DATABASE_EXPORT_EXPORT JDObjectRegistry
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

            static const JDObject& getObjectDefinition(const JsonObject& json);
            static std::string getObjectTypeString(const JsonObject& json);

            static const JDObject& getObjectDefinition(const std::string& className);

        private:
            std::map<std::string, JDObject> m_registry;
        };
    }
}