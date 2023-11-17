#pragma once

#include "JD_base.h"
#include "JDDeclaration.h"


#include <string>
#include <vector>
#ifdef JD_USE_QJSON
#include <QJsonObject>
#else
#include "Json/JsonValue.h"
#endif

namespace JsonDatabase
{
    namespace Internal
    {
        class JsonUtilities
        {
        public:
#ifdef JD_USE_QJSON
            static bool getJsonArray(const std::vector<JDObjectInterface*>& objs, std::vector<QJsonObject>& jsonOut);
            static bool getJsonArray(const std::vector<JDObjectInterface*>& objs, std::vector<QJsonObject>& jsonOut, 
                                     WorkProgress* progress, double deltaProgress);
            static bool serializeObject(JDObjectInterface* obj, std::string& serializedOut);
            static bool serializeJson(const QJsonObject& obj, std::string& serializedOut);

            static bool deserializeJson(const QJsonObject& json, JDObjectInterface* objOriginal, JDObjectInterface*& objOut);
            static bool deserializeOverrideFromJson(const QJsonObject& json, JDObjectInterface* obj, bool& hasChangedOut);
            static bool deserializeOverrideFromJson(const QJsonObject& json, JDObjectInterface* obj);
#else
            static bool serializeObject(JDObjectInterface* obj, std::string& serializedOut);
            static bool deserializeJson(const JsonValue& json, JDObjectInterface* objOriginal, JDObjectInterface*& objOut);

#endif
        };
    }
}