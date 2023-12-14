#include "utilities/JDSerializable.h"

namespace JsonDatabase
{
    namespace Utilities
    {
#if JD_ACTIVE_JSON == JD_JSON_QT
        bool JDSerializable::getJsonValue(const QJsonObject& obj, QJsonObject& value, const QString& key)
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
        bool JDSerializable::getJsonValue(const JsonObject& obj, JsonObject& value, const std::string& key)
#endif
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_5);
            if (obj.contains(key))
            {
                JD_GENERAL_PROFILING_BLOCK("contains key", JD_COLOR_STAGE_6);
#if JD_ACTIVE_JSON == JD_JSON_QT
                QJsonValue val = obj[key];
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
                const JsonValue& val = obj.find(key)->second;
#endif
#if JD_ACTIVE_JSON == JD_JSON_QT
                if (val.isObject())
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
                if (val.holds<JsonObject>())
#endif
                {
                    JD_GENERAL_PROFILING_BLOCK("is object", JD_COLOR_STAGE_7);
#if JD_ACTIVE_JSON == JD_JSON_QT
                    value = val.toObject();
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
                    value = val.get<JsonObject>();
#endif
                    return true;
                }
                else
                    return false;
            }
            return false;
        }
#if JD_ACTIVE_JSON == JD_JSON_QT
        bool JDSerializable::getJsonValue(const QJsonObject& obj, QVariant& value, const QString& key)
        {
            if (obj.contains(key))
            {
                QJsonValue val = obj[key];
                value = val.toVariant();
                return true;
            }
            return false;
        }
#endif
#if JD_ACTIVE_JSON == JD_JSON_QT
        bool JDSerializable::getJsonValue(const QJsonObject& obj, QString& value, const QString& key)
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
        bool JDSerializable::getJsonValue(const JsonObject& obj, QString& value, const std::string& key)
#endif
        {
            if (obj.contains(key))
            {
#if JD_ACTIVE_JSON == JD_JSON_QT
                value = obj[key].toString();
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL 
                value = QString::fromStdString(obj.find(key)->second.get<std::string>());
#endif
                return true;
            }
            return false;
        }
#if JD_ACTIVE_JSON == JD_JSON_QT
        bool JDSerializable::getJsonValue(const QJsonObject& obj, std::string& value, const QString& key)
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
        bool JDSerializable::getJsonValue(const JsonObject& obj, std::string& value, const std::string& key)
#endif
        {
            if (obj.contains(key))
            {
#if JD_ACTIVE_JSON == JD_JSON_QT
                value = obj[key].toString().toStdString();
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
                value = obj.find(key)->second.get<std::string>();
#endif
                return true;
            }
            return false;
        }

#if JD_ACTIVE_JSON == JD_JSON_QT
        bool JDSerializable::getJsonValue(const QJsonObject& obj, int& value, const QString& key)
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
        bool JDSerializable::getJsonValue(const JsonObject& obj, int& value, const std::string& key)
#endif
        {
            if (obj.contains(key))
            {
#if JD_ACTIVE_JSON == JD_JSON_QT
                value = obj[key].toInt(value);
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
                value = obj.find(key)->second.get<int>();
#endif
                return true;
            }
            return false;
        }
        /*
        #if JD_ACTIVE_JSON == JD_JSON_QT
        bool JDSerializable::getJsonValue(const QJsonObject& obj, JDObjectID::IDType& value, const QString& key)
        #elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
        bool JDSerializable::getJsonValue(const JsonObject& obj, JDObjectID::IDType& value, const std::string& key)
        #endif
        {
            if (obj.contains(key))
            {
        #if JD_ACTIVE_JSON == JD_JSON_QT
                value = obj[key].toInt(value);
        #elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
                value = obj.find(key)->second.getInt();
        #endif
                return true;
            }
            return false;
        }
        */
#if JD_ACTIVE_JSON == JD_JSON_QT
        bool JDSerializable::getJsonValue(const QJsonObject& obj, double& value, const QString& key)
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
        bool JDSerializable::getJsonValue(const JsonObject& obj, double& value, const std::string& key)
#endif
        {
            if (obj.contains(key))
            {
#if JD_ACTIVE_JSON == JD_JSON_QT
                value = obj[key].toDouble(value);
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
				value = obj.find(key)->second.get<double>();
#endif
                return true;
            }
            return false;
        }

#if JD_ACTIVE_JSON == JD_JSON_QT
        bool JDSerializable::getJsonValue(const QJsonObject& obj, float& value, const QString& key)
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
        bool JDSerializable::getJsonValue(const JsonObject& obj, float& value, const std::string& key)
#endif
        {
            if (obj.contains(key))
            {
#if JD_ACTIVE_JSON == JD_JSON_QT
                value = static_cast<float>(obj[key].toDouble(static_cast<double>(value)));
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
                value = static_cast<float>(obj.find(key)->second.get<double>());
#endif
                return true;
            }
            return false;
        }
#if JD_ACTIVE_JSON == JD_JSON_QT
        bool JDSerializable::getJsonValue(const QJsonObject& obj, bool& value, const QString& key)
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
        bool JDSerializable::getJsonValue(const JsonObject& obj, bool& value, const std::string& key)
#endif
        {
            if (obj.contains(key))
            {
#if JD_ACTIVE_JSON == JD_JSON_QT
                value = obj[key].toBool(value);
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
				value = obj.find(key)->second.get<bool>();
#endif
                return true;
            }
            return false;
        }

    }
}