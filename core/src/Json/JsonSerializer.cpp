#include "Json/JsonSerializer.h"
#include <iomanip> // For std::setprecision
#include <limits> // For std::numeric_limits


namespace JsonDatabase
{

    std::string JsonSerializer::serializeValue(const JsonValue& value)
    {
        std::string result;
        switch (value.m_type)
        {
        case JsonValue::Type::Null:
            result = serializeNull(); break;
        case JsonValue::Type::String:
            result = serializeString(std::get<std::string>(value.m_value)); break;
        case JsonValue::Type::Int:
            result = serializeInt(std::get<int>(value.m_value)); break;
        case JsonValue::Type::Double:
            result = serializeDouble(std::get<double>(value.m_value)); break;
        case JsonValue::Type::Bool:
            result = serializeBool(std::get<bool>(value.m_value)); break;
        case JsonValue::Type::Array:
            result = serializeArray(std::get<JsonArray>(value.m_value)); break;
        case JsonValue::Type::Object:
            result = serializeObject(std::get<JsonObject>(value.m_value)); break;
        }
       
        /*if (m_useNewLines)
		{
            result += "\n";
		}*/
        return result;
    }

    std::string JsonSerializer::serializeObject(const JsonObject& object)
    {
        std::string result = "{";
        std::string spaced = std::string(m_useSpaces, ' ');
        if (m_useNewLines)
        {
            result += "\n";
            m_indent++;
        }
        bool first = true;
        for (const auto& pair : object) {
            if (!first) {
                result += ",";
                if (m_useNewLines)
                    result += "\n";
            }
            first = false;
            result += std::string(m_indent, '\t') + "\"" + pair.first + "\""+ spaced+":"+ spaced + serializeValue(pair.second);
        }
        if (m_useNewLines)
        {
            result += "\n";
            m_indent--;
        }
        result += std::string(m_indent, '\t') + "}";
        return result;
    }

    std::string JsonSerializer::serializeArray(const JsonArray& array)
    {
        std::string result = "[";
        if (m_useNewLines)
        {
            result += "\n";
            m_indent++;
        }
        bool first = true;
        for (const auto& value : array) {
            if (!first) {
                result += ",";
                if (m_useNewLines)
                    result += "\n";
            }
            first = false;
            result += std::string(m_indent, '\t') + serializeValue(value);
        }
        if (m_useNewLines)
        {
            result += "\n";
            m_indent--;
        }
        result += std::string(m_indent, '\t') + "]";
        return result;
    }

    std::string JsonSerializer::serializeString(const std::string& str)
    {
        return escapeString(str);
    }
    std::string JsonSerializer::serializeInt(int value)
    {
        return std::to_string(value);
    }
    std::string JsonSerializer::serializeDouble(double value) 
    {
        /*
            Formats:
            3.14159265358916562
            3.14159265358899997
            3.25
            3.0
            32.0
            321.0
            321.0009765625
            0.0
        */
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << value;

        std::string result = oss.str();
        size_t dotPos = result.find('.');
        if (dotPos != std::string::npos) {
            // Find position of last non-zero digit after the dot
            size_t lastNonZero = result.find_last_not_of('0');
            if (lastNonZero != std::string::npos)
            {
                if (result[lastNonZero] == '.')
                    result.resize(lastNonZero + 2); // Truncate extra zeros after the dot
                else
                    result.resize(lastNonZero + 1); // Truncate extra zeros after the dot
            }
        }
        else {
            result += ".0"; // Add a decimal point and a trailing zero if there's no decimal part
        }
        return result;
    }
    std::string JsonSerializer::serializeBool(bool value)
    {
        return value ? "true" : "false";
    }
    const std::string &JsonSerializer::serializeNull()
    {
        static const std::string null = "null";
        return null;
    }
    std::string JsonSerializer::escapeString(const std::string& str)
    {
        size_t size = str.size();
        for (char c : str) 
        {
			switch (c) 
            {
			    case '"':
			    case '\\':
			    case '\b':
			    case '\f':
			    case '\n':
			    case '\r':
			    case '\t':
				    size += 2;
				    break;
			    default:
				    break;
			}
		}
        std::string result(size+2, '\0');
        result[0] = '"';
        result[size+1] = '"';
        size_t pos = 1;
        for (char c : str) 
        {
			switch (c) 
            {
			case '"':
				result[pos++] = '\\';
				result[pos++] = '"';
				break;
			case '\\':
				result[pos++] = '\\';
				result[pos++] = '\\';
				break;
			case '\b':
				result[pos++] = '\\';
				result[pos++] = 'b';
				break;
			case '\f':
				result[pos++] = '\\';
				result[pos++] = 'f';
				break;
			case '\n':
				result[pos++] = '\\';
				result[pos++] = 'n';
				break;
			case '\r':
				result[pos++] = '\\';
				result[pos++] = 'r';
				break;
			case '\t':
				result[pos++] = '\\';
				result[pos++] = 't';
				break;
			default:
				result[pos++] = c;
				break;
			}
		}
		return result;
    }
}