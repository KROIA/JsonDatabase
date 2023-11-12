#pragma once

#include "JD_base.h"
#include "JDDeclaration.h"

#include <string>
#include <vector>
#include <QJsonObject>

namespace JsonDatabase
{
    namespace Internal
    {
        class JsonUtilities
        {
        public:
            static bool getJsonArray(const std::vector<JDObjectInterface*>& objs, std::vector<QJsonObject>& jsonOut);
            static bool serializeObject(JDObjectInterface* obj, std::string& serializedOut);
            static bool serializeJson(const QJsonObject& obj, std::string& serializedOut);

            static bool deserializeJson(const QJsonObject& json, JDObjectInterface* objOriginal, JDObjectInterface*& objOut);
            static bool deserializeOverrideFromJson(const QJsonObject& json, JDObjectInterface* obj, bool& hasChangedOut);
            static bool deserializeOverrideFromJson(const QJsonObject& json, JDObjectInterface* obj);
        };
    }
}