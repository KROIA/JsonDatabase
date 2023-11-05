#pragma once

#include "JD_base.h"
#include "JDObjectInterface.h"
#include <map>

namespace JsonDatabase
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

    private:
        std::map<std::string, JDObjectInterface*> m_registry;
    };
}