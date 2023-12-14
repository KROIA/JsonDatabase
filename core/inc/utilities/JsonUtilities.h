#pragma once

#include "JD_base.h"
#include "JDDeclaration.h"


#include <string>
#include <vector>
#if JD_ACTIVE_JSON == JD_JSON_QT
#include <QJsonObject>
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
#include "Json/JsonValue.h"
#endif

namespace JsonDatabase
{
    namespace Utilities
    {
        class JsonUtilities
        {
        public:
#if JD_ACTIVE_JSON == JD_JSON_QT
           // static bool getJsonArray(const std::vector<JDObject>& objs, std::vector<QJsonObject>& jsonOut);
           // static bool getJsonArray(const std::vector<JDObject>& objs, std::vector<QJsonObject>& jsonOut, 
           //                          WorkProgress* progress, double deltaProgress);
            //static bool serializeObject(JDObject obj, std::string& serializedOut);
            //static bool serializeJson(const QJsonObject& obj, std::string& serializedOut);

          /*  static bool deserializeJson(
                const QJsonObject& json, 
                JDObject objOriginal, 
                JDObject& objOut, 
                JDObjectIDDomain& idDomain,
                JDManager& manager);*/
         //   static bool deserializeOverrideFromJson(const QJsonObject& json, JDObject obj, bool& hasChangedOut);
         //   static bool deserializeOverrideFromJson(const QJsonObject& json, JDObject obj);
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
           // static bool getJsonArray(const std::vector<JDObject>& objs, JsonArray& jsonOut);
           // static bool getJsonArray(const std::vector<JDObject>& objs, JsonArray& jsonOut,
           //     WorkProgress* progress, double deltaProgress);

          //  static bool serializeObject(JDObject obj, std::string& serializedOut);
          /*  static bool deserializeJson(
                const JsonValue& json, 
                JDObject objOriginal, 
                JDObject& objOut,
                JDObjectIDDomain& idDomain,
                JDManager &manager);*/

           // static bool deserializeOverrideFromJson(const JsonValue& json, JDObject obj, bool& hasChangedOut);
           // static bool deserializeOverrideFromJson(const JsonValue& json, JDObject obj);
#endif
        };
    }
}