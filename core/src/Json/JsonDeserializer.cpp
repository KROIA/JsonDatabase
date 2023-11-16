#include "Json/JsonDeserializer.h"

namespace JsonDatabase
{



    JsonValue JsonDeserializer::deserializeValue(const std::string& json) 
    {
        std::string jsonString;
        nornmalizeJsonString(json, jsonString);
        size_t index = 0;
        switch (jsonString[index]) {
        case '{':
            return deserializeObject_internal(jsonString);
        case '[':
            return deserializeArray_internal(jsonString);
        case '"':
            return JsonValue(readString(jsonString, index));
        case 't':
        case 'f':
            return JsonValue(readBool(jsonString, index));
        case 'n':
            return JsonValue();
        default:
            int intValue = 0;
            bool isInt = false;
            double number = readNumber(jsonString, index, intValue, isInt);
            if(isInt)
				return JsonValue(intValue);
            return JsonValue(number);
        }
    }

    JsonValue JsonDeserializer::deserializeObject(const std::string& json) 
    {
        std::string jsonString;
        nornmalizeJsonString(json, jsonString);
        JsonObject object;
        size_t index = 1; // Skip the '{' character
        while (jsonString[index] != '}') {
            auto [key, value] = parsePair(jsonString, index);
            object[key] = value;
            index++; // Move to the next character or skip ',' or '}'
        }
        return JsonValue(object);
    }

    JsonValue JsonDeserializer::deserializeArray(const std::string& json) 
    {
        std::string jsonString;
        nornmalizeJsonString(json, jsonString);
        JsonArray array;
        size_t index = 1; // Skip the '[' character
        while (jsonString[index] != ']') {
            auto value = deserializeValue_internal(jsonString.substr(index));
            array.push_back(value);
            index++; // Move to the next character or skip ',' or ']'
        }
        return JsonValue(array);
    }

    JsonValue JsonDeserializer::deserializeValue_internal(const std::string& json) 
    {
        size_t index = 0;
        switch (json[index]) {
        case '{':
            return deserializeObject_internal(json);
        case '[':
            return deserializeArray_internal(json);
        case '"':
            return JsonValue(readString(json, index));
        case 't':
        case 'f':
            return JsonValue(readBool(json, index));
        case 'n':
            return JsonValue();
        default:
            int intValue = 0;
            bool isInt = false;
            double number = readNumber(json, index, intValue, isInt);
            if (isInt)
                return JsonValue(intValue);
            return JsonValue(number);
        }
    }

    JsonValue JsonDeserializer::deserializeObject_internal(const std::string& json) {
        JsonObject object;
        size_t index = 1; // Skip the '{' character
        skipWhiteSpace(json, index);
        while (json[index] != '}') {
            auto [key, value] = parsePair(json, index);
            object[key] = value;
            index++; // Move to the next character or skip ',' or '}'
        }
        return JsonValue(object);
    }

    JsonValue JsonDeserializer::deserializeArray_internal(const std::string& json) {
        JsonArray array;
        size_t index = 1; // Skip the '[' character
        while (json[index] != ']') {
            auto value = deserializeValue_internal(json.substr(index));
            array.push_back(value);
            index++; // Move to the next character or skip ',' or ']'
        }
        return JsonValue(array);
    }

    std::pair<std::string, JsonValue> JsonDeserializer::parsePair(const std::string& json, size_t& index) {
        index++; // Skip the opening double quote
        std::string key = readString(json, index);
        index++; // Skip the closing double quote
        index++; // Skip the colon ':'
        JsonValue value = deserializeValue_internal(json.substr(index));
        index++; // Move to the next character or skip ',' or '}'
        return { key, value };
    }

    std::string JsonDeserializer::readString(const std::string& json, size_t& index) {
        index++; // Skip the opening double quote
        size_t start = index;
        while (json[index] != '"') {
            index++;
        }
        std::string str = json.substr(start, index - start);
        index++; // Skip the closing double quote
        return str;
    }

    double JsonDeserializer::readNumber(const std::string& json, size_t& index, int& intValue, bool& isInt)
    {
        size_t start = index;
        while (json[index] != ',' && json[index] != '}' && json[index] != ']') {
            index++;
        }
        std::string str = json.substr(start, index - start);
        if(str.find(".") != std::string::npos)
		{
			isInt = false;
			return std::stod(str);
		}
		else
		{
			isInt = true;
			intValue = std::stoi(str);
		}
        return 0;
    }

    bool JsonDeserializer::readBool(const std::string& json, size_t& index) {
        std::string str;
        if (json[index] == 't') {
            str = json.substr(index, 4); // true
            index += 4;
            return true;
        }
        else {
            str = json.substr(index, 5); // false
            index += 5;
            return false;
        }
    }

    void JsonDeserializer::skipWhiteSpace(const std::string& jsonString, size_t &index) {
        while (index < jsonString.size() && std::isspace(jsonString[index])) {
            index++;
        }
    }
    void JsonDeserializer::nornmalizeJsonString(const std::string& jsonString, std::string& jsonStringOut)
    {
		std::vector<char> chars = { '\n', '\t', '\r' };
		removeChars(jsonString, jsonStringOut, chars);
    }
    void JsonDeserializer::removeChars(const std::string& jsonString, std::string& jsonStringOut,
        const std::vector<char>& chars)
    {
        jsonStringOut.resize(jsonString.size(), '\0');
        size_t count = 0;
        for (size_t i = 0; i < jsonString.size(); ++i)
        {
            for(size_t j = 0; j < chars.size(); ++j)
			{
				if (jsonString[i] == chars[j])
				{
					goto skip;
				}
			}
            jsonStringOut[count] = jsonString[i];
            skip:;
        }
    }
	/*JsonValue JsonDeserializer::deserialize(const std::string& jsonString) {
		position = 0; // Initialize the position
		return parseValue(jsonString);
	}


    JsonValue JsonDeserializer::parseValue(const std::string& jsonString) {
        skipWhiteSpace(jsonString);

        char currentChar = jsonString[position];
        if (currentChar == '{') {
            return parseObject(jsonString);
        }
        else if (currentChar == '[') {
            return parseArray(jsonString);
        }
        else if (currentChar == '"') {
            return parseString(jsonString);
        }
        else if (currentChar == 't' || currentChar == 'f') {
            return parseBoolean(jsonString);
        }
        else if (currentChar == 'n') {
            return parseNull(jsonString);
        }
        else if (std::isdigit(currentChar) || currentChar == '-') {
            return parseNumber(jsonString);
        }
        else {
            // Throw error or handle invalid JSON
            std::cerr << "Invalid JSON format!" << std::endl;
            return JsonValue(); // Return a null JsonValue
        }
    }

    void JsonDeserializer::skipWhiteSpace(const std::string& jsonString) {
        while (position < jsonString.size() && std::isspace(jsonString[position])) {
            position++;
        }
    }

    JsonValue JsonDeserializer::parseObject(const std::string& jsonString) {
        JsonValue object;
        object.m_type = JsonValue::Type::Object;

        size_t pos = jsonString.find('{');
        if (pos == std::string::npos) {
            // Invalid JSON object
            std::cerr << "Invalid JSON object!" << std::endl;
            return object;
        }

        size_t endPos = jsonString.find('}', pos + 1);
        if (endPos == std::string::npos) {
            // Invalid JSON object
            std::cerr << "Invalid JSON object!" << std::endl;
            return object;
        }

        std::string objectContent = jsonString.substr(pos + 1, endPos - pos - 1);
        // Parse key-value pairs
        // Implement your logic here for parsing key-value pairs inside the object

        return object;
    }

    JsonValue JsonDeserializer::parseArray(const std::string& jsonString) {
        JsonValue array;
        array.m_type = JsonValue::Type::Array;

        size_t pos = jsonString.find('[');
        if (pos == std::string::npos) {
            // Invalid JSON array
            std::cerr << "Invalid JSON array!" << std::endl;
            return array;
        }

        size_t endPos = jsonString.find(']', pos + 1);
        if (endPos == std::string::npos) {
            // Invalid JSON array
            std::cerr << "Invalid JSON array!" << std::endl;
            return array;
        }

        std::string arrayContent = jsonString.substr(pos + 1, endPos - pos - 1);
        // Parse array elements
        // Implement your logic here for parsing array elements

        return array;
    }

    JsonValue JsonDeserializer::parseString(const std::string& jsonString) {
        JsonValue stringValue;
        stringValue.m_type = JsonValue::Type::String;

        // Implement your logic here for parsing a string value

        return stringValue;
    }

    JsonValue JsonDeserializer::parseBoolean(const std::string& jsonString) {
        JsonValue boolValue;
        boolValue.m_type = JsonValue::Type::Bool;

        // Implement your logic here for parsing a boolean value

        return boolValue;
    }

    JsonValue JsonDeserializer::parseNull(const std::string& jsonString) {
        JsonValue nullValue;
        nullValue.m_type = JsonValue::Type::Null;

        // Implement your logic here for parsing a null value

        return nullValue;
    }

    JsonValue JsonDeserializer::parseInt(const std::string& jsonString) {
        JsonValue numberValue;
        numberValue.m_type = JsonValue::Type::Int;

        // Implement your logic here for parsing a numeric value

        return numberValue;
    }
    JsonValue JsonDeserializer::parseDouble(const std::string& jsonString) {
        JsonValue numberValue;
        numberValue.m_type = JsonValue::Type::Double;

        // Implement your logic here for parsing a numeric value

        return numberValue;
    }*/
}