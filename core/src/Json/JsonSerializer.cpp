#include "Json/JsonSerializer.h"

namespace JsonDatabase
{

    std::string JsonSerializer::serializeValue(const JsonValue& value)
    {
        switch (value.m_type)
        {
        case JsonValue::Type::Null:
            return "null";
        case JsonValue::Type::String:
            return std::get<std::string>(value.m_value);
        case JsonValue::Type::Int:
            return std::to_string(std::get<int>(value.m_value));
        case JsonValue::Type::Double:
            return std::to_string(std::get<double>(value.m_value));
        case JsonValue::Type::Bool:
            return std::get<bool>(value.m_value) ? "true" : "false";
        case JsonValue::Type::Array:
            return serializeArray(std::get<JsonArray>(value.m_value));
        case JsonValue::Type::Object:
            return serializeObject(std::get<JsonObject>(value.m_value));
        }
        return ""; // Return empty string if type is not recognized
    }

    std::string JsonSerializer::serializeObject(const JsonObject& object)
    {
        std::string result = "{";
        bool first = true;
        for (const auto& pair : object) {
            if (!first) {
                result += ",";
            }
            first = false;
            result += "\"" + pair.first + "\":" + serializeValue(pair.second);
        }
        result += "}";
        return result;
    }

    std::string JsonSerializer::serializeArray(const JsonArray& array)
    {
        std::string result = "[";
        bool first = true;
        for (const auto& value : array) {
            if (!first) {
                result += ",";
            }
            first = false;
            result += serializeValue(value);
        }
        result += "]";
        return result;
    }
}