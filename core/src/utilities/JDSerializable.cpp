#include "utilities/JDSerializable.h"

namespace JsonDatabase
{
    namespace Utilities
    {
        bool JDSerializable::getJsonValue(const JsonObject& obj, JsonObject& value, const std::string& key)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_5);
			const auto& it = obj.find(key);
			if (it != obj.end())
            {
                JD_GENERAL_PROFILING_BLOCK("contains key", JD_COLOR_STAGE_6);
                const JsonValue& val = it->second;
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
        bool JDSerializable::getJsonValue(const JsonObject& obj, JsonArray& value, const std::string& key)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_5);
			const auto& it = obj.find(key);
            if (it != obj.end())
            {
                JD_GENERAL_PROFILING_BLOCK("contains key", JD_COLOR_STAGE_6);
                const JsonValue& val = it->second;
                if (val.holds<JsonArray>())
                {
                    JD_GENERAL_PROFILING_BLOCK("is array", JD_COLOR_STAGE_7);
                    value = val.get<JsonArray>();
                    return true;
                }
                else
                    return false;
            }
            return false;
        }
        bool JDSerializable::getJsonValue(const JsonObject& obj, QString& value, const std::string& key)
        {
			const auto& it = obj.find(key);
            if (it != obj.end())
            {
                value = QString::fromStdString(it->second.get<std::string>());
                return true;
            }
            return false;
        }
        bool JDSerializable::getJsonValue(const JsonObject& obj, std::string& value, const std::string& key)
        {
			const auto& it = obj.find(key);
            if (it != obj.end())
            {
                value = it->second.get<std::string>();
                return true;
            }
            return false;
        }

        bool JDSerializable::getJsonValue(const JsonObject& obj, long& value, const std::string& key)
        {
            const auto& it = obj.find(key);
            if (it != obj.end())
            {
                value = it->second.get<long>();
                return true;
            }
            return false;
        }


        bool JDSerializable::getJsonValue(const JsonObject& obj, double& value, const std::string& key)
        {
            const auto& it = obj.find(key);
            if (it != obj.end())
            {
				value = it->second.get<double>();
                return true;
            }
            return false;
        }

        bool JDSerializable::getJsonValue(const JsonObject& obj, float& value, const std::string& key)
        {
            const auto& it = obj.find(key);
            if (it != obj.end())
            {
                value = static_cast<float>(it->second.get<double>());
                return true;
            }
            return false;
        }
        bool JDSerializable::getJsonValue(const JsonObject& obj, bool& value, const std::string& key)
        {
            const auto& it = obj.find(key);
            if (it != obj.end())
            {
				value = it->second.get<bool>();
                return true;
            }
            return false;
        }

    }
}