#include "utilities/JDSerializable.h"

namespace JsonDatabase
{
    namespace Utilities
    {
        bool JDSerializable::getJsonValue(const JsonObject& obj, JsonObject& value, const std::string& key)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_5);
            if (obj.contains(key))
            {
                JD_GENERAL_PROFILING_BLOCK("contains key", JD_COLOR_STAGE_6);
                const JsonValue& val = obj.find(key)->second;
                if (val.holds<JsonObject>())

                {
                    JD_GENERAL_PROFILING_BLOCK("is object", JD_COLOR_STAGE_7);
                    value = val.get<JsonObject>();
                    return true;
                }
                else
                    return false;
            }
            return false;
        }
        bool JDSerializable::getJsonValue(const JsonObject& obj, QString& value, const std::string& key)
        {
            if (obj.contains(key))
            {
                value = QString::fromStdString(obj.find(key)->second.get<std::string>());
                return true;
            }
            return false;
        }
        bool JDSerializable::getJsonValue(const JsonObject& obj, std::string& value, const std::string& key)
        {
            if (obj.contains(key))
            {
                value = obj.find(key)->second.get<std::string>();
                return true;
            }
            return false;
        }

        bool JDSerializable::getJsonValue(const JsonObject& obj, long& value, const std::string& key)
        {
            if (obj.contains(key))
            {
                value = obj.find(key)->second.get<long>();
                return true;
            }
            return false;
        }


        bool JDSerializable::getJsonValue(const JsonObject& obj, double& value, const std::string& key)
        {
            if (obj.contains(key))
            {
				value = obj.find(key)->second.get<double>();
                return true;
            }
            return false;
        }

        bool JDSerializable::getJsonValue(const JsonObject& obj, float& value, const std::string& key)
        {
            if (obj.contains(key))
            {
                value = static_cast<float>(obj.find(key)->second.get<double>());
                return true;
            }
            return false;
        }
        bool JDSerializable::getJsonValue(const JsonObject& obj, bool& value, const std::string& key)
        {
            if (obj.contains(key))
            {
				value = obj.find(key)->second.get<bool>();
                return true;
            }
            return false;
        }

    }
}