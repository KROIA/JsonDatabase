#include "Json/JsonDeserializer.h"

namespace JsonDatabase
{



    JsonValue JsonDeserializer::deserializeValue(const std::string& json) 
    {
        std::string normalized;
        size_t index = 0;
        nornmalizeJsonString(json, normalized);
        return deserializeValue_internal(normalized, index);
    }

    JsonValue JsonDeserializer::deserializeObject(const std::string& json) 
    {
        std::string jsonString;
        size_t index = 0;
        nornmalizeJsonString(json, jsonString);
        return deserializeObject_internal(jsonString, index);
        /*
        JsonObject object;
        size_t index = 1; // Skip the '{' character
        while (jsonString[index] != '}') {
            auto [key, value] = deserializePair(jsonString, index);
            object[key] = value;
            index++; // Move to the next character or skip ',' or '}'
        }
        return JsonValue(object);*/
    }

    JsonValue JsonDeserializer::deserializeArray(const std::string& json) 
    {
        std::string jsonString;
        size_t index = 0;
        nornmalizeJsonString(json, jsonString);
        return deserializeArray_internal(jsonString, index);
        /*JsonArray array;
        size_t index = 1; // Skip the '[' character
        while (jsonString[index] != ']') {
            auto value = deserializeValue_internal(jsonString, index);
            array.push_back(value);
            index++; // Move to the next character or skip ',' or ']'
        }
        return JsonValue(array);*/
    }

    JsonValue JsonDeserializer::deserializeValue_internal(const std::string& json, size_t& index)
    {
        switch (json[index]) {
        case '{':
            return deserializeObject_internal(json, index);
        case '[':
            return deserializeArray_internal(json, index);
        case '"':
        {
            JsonValue val(deserializeString(json, index));
            //++index; // Skip the comma
            return val;
        }
        case 't':
        case 'f':
            return JsonValue(deserializeBool(json, index));
        case 'n':
            index += 4; // Skip the "null" keyword
            return JsonValue();
        default:
            int intValue = 0;
            double doubleValue = 0;
            
            int result = deserializeNumber(json, intValue, doubleValue, index);
            //++index; // Skip the comma
            if (result == 1)
                return JsonValue(intValue);

            return JsonValue(doubleValue);
        }
    }

    JsonValue JsonDeserializer::deserializeObject_internal(const std::string& json, size_t& index) {
        JsonObject object;
        index++; // Skip the '{' character
        //skipWhiteSpace(json, index);
        while (json[index] != '}') {
            auto [key, value] = deserializePair(json, index);
            object[key] = value;
            if (json[index] == '}')
                break;
            index++; // Move to the next character or skip ',' or '}'
        }
        index++; // Skip the '}' character
        return JsonValue(object);
    }

    JsonValue JsonDeserializer::deserializeArray_internal(const std::string& json, size_t& index) {
        JsonArray array;
        index++; // Skip the '[' character
        while (json[index] != ']') {
            auto value = deserializeValue_internal(json, index);
            array.push_back(value);
            // if (json[index] != ']')
            // {
            //     index++; // Move to the next character or skip ',' or ']'
            //     break;
            // }
            if (json[index] == ']')
                break;
             index++; // Move to the next character or skip ',' or ']'
        }
        index++; // Skip the ']' character
        return JsonValue(array);
    }

    std::pair<std::string, JsonValue> JsonDeserializer::deserializePair(const std::string& json, size_t& index) {
        //index++; // Skip the opening double quote
        std::string key = deserializeString(json, index);
        //index++; // Skip the closing double quote
        index++; // Skip the colon ':'
        JsonValue value = deserializeValue_internal(json, index);
        //index++; // Move to the next character or skip ',' or '}'
        return { key, value };
    }

    std::string JsonDeserializer::deserializeString(const std::string& json, size_t& index) {
        index++; // Skip the opening double quote
        size_t start = index;
        bool isString = true;
        bool lastCharWasNotEscape = true;
        while (isString)
        {
            char currentChar = json[index];
            bool currentCharIsStringKey = currentChar == '"' && lastCharWasNotEscape;

            if (currentCharIsStringKey)
            {
                isString = false;
                break;
            }
            index++;

            lastCharWasNotEscape = currentChar != '\\';
        }
        std::string str = unescapeString(json.substr(start, index - start));
        index++; // Skip the closing double quote
        return str;
    }

    /*double JsonDeserializer::deserializeNumber(const std::string& json, size_t& index, int& intValue, bool& isInt)
    {

        std::istringstream iss(json);
        double result;
        if (!(iss >> result)) {
            // Handle invalid input
            
        }
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
    }*/


    int JsonDeserializer::deserializeNumber(const std::string& str, int& intValue, double& doubleValue, size_t& index)
    {
        intValue = 0;
        doubleValue = 0;
        std::size_t found = str.find_first_not_of("-0123456789.eE", index);
        std::string subStr;
        if (found != std::string::npos)
        {
            subStr = str.substr(index, found - index);
            index += found - index;
            //std::cout << "The first non-alphabetic character is " << str[found1];
            //std::cout << " at position " << found1 << '\n';
        }
        else
        {
            subStr = str.substr(index);
            index++;
        }
        if (found == 0 || subStr.size() == 0)
        {
            return 0;
        }

        /*
        
        size_t firstDashPos = subStr.find("-");
        size_t firstDotPos = subStr.find(".");


        // "-" must be at the first position
        // "." can't be at the first position
        if ((firstDashPos != std::string::npos && firstDashPos != 0) ||
            (firstDotPos != std::string::npos && (firstDotPos == 0 || firstDashPos == firstDotPos - 1)))
        {
            // ".56"; "5-";
            return 0;
        }

        size_t firstEpos1 = subStr.find("e");
        size_t firstEpos2 = subStr.find("E");
        size_t firstEpos = std::min(firstEpos1, firstEpos2);
        // if (firstEpos != std::string::npos)
        // {
        //     if (firstEpos < firstDashPos && firstDashPos != std::string::npos)
        //     {
        //         subStr = subStr.substr(0, firstEpos);
        //     }
        //     if (firstEpos < firstDotPos && firstDotPos != std::string::npos)
        //     {
        // 
        //     }
        // }


        // shrink the string to contain only one "-" and "."
        size_t secondDashPos = std::string::npos;
        size_t secondDotPos = std::string::npos;
        if (firstDashPos != std::string::npos)
            secondDashPos = subStr.find("-", firstDashPos + 1);
        if (firstDotPos != std::string::npos)
            secondDotPos = subStr.find(".", firstDotPos + 1);

        if (secondDashPos != std::string::npos ||
            secondDotPos != std::string::npos)
        {
            // "0.65.0" --> "0.65";  "-0.65-.5" --> "-0.65"
            size_t lower = std::min(secondDashPos, secondDotPos);
            subStr = subStr.substr(0, lower);
        }
        // if(firstDashPos != std::string::npos && subStr.size())


        // int dotCount = std::count(subStr.begin(),subStr.end(),'.');
        
         // Detects "0.", "0.a"
        std::size_t dotP = subStr.find(".");
        if (dotP != std::string::npos)
        {
            if (dotP == subStr.size() - 1)
            {
               // std::cout << "invalid dot format: \"" << subStr << "\" \"" << str << "\"\n";
            }
        }

        std::size_t found2 = str.find_first_not_of("-.");
        if (subStr.size())
        {
            // ---a5

        }*/

        std::size_t dotP = subStr.find(".");
        std::istringstream iss(subStr);
        if (dotP == std::string::npos)
        {
            iss >> intValue;
            return 1;
        }
        else
        {
            iss >> std::setprecision(std::numeric_limits<double>::max_digits10) >> doubleValue;
        }
        return 2;
    }

    bool JsonDeserializer::deserializeBool(const std::string& json, size_t& index) {
        std::string str;
        if (json[index] == 't') {
            //str = json.substr(index, 4); // true
            index += 4;
            return true;
        }
        else {
            //str = json.substr(index, 5); // false
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
		std::vector<char> chars = { '\n', '\t', '\r', ' '};
		removeChars(jsonString, jsonStringOut, chars);
    }
    void JsonDeserializer::removeChars(const std::string& jsonString, std::string& jsonStringOut,
        const std::vector<char>& chars)
    {
        jsonStringOut.resize(jsonString.size(), '\0');
        size_t count = 0;
        bool isString = false;
        bool lastCharWasNotEscape = true;

        /*if (jsonString.size() > 2)
        {
            if (jsonString[0] == '\\' && jsonString[1] == '"')
                isString = true;
        }*/

        for (size_t i = 0; i < jsonString.size(); ++i)
        {
            char currentChar = jsonString[i];
            bool currentCharIsStringKey = currentChar == '"' && lastCharWasNotEscape;

            if (currentCharIsStringKey)
            {
                isString = !isString;
            }
            
            if (!isString)
            {
                // If in string, ignore the remove chars command
                for (size_t j = 0; j < chars.size(); ++j)
                {
                    if (currentChar == chars[j])
                    {
                        goto skip;
                    }
                }
            }
            jsonStringOut[count++] = currentChar;
            skip:;
            lastCharWasNotEscape = currentChar != '\\';
        }
    }

    std::string JsonDeserializer::unescapeString(const std::string& str)
    {
        // remove dummy escape symbols and replace them with the real ones
        std::string result;
        result.reserve(str.size());

        for (size_t i = 0; i < str.size(); ++i) {
            if (str[i] == '\\') {
                if (i + 1 < str.size()) {
                    switch (str[i + 1]) {
                    case '"':
                        result += '"';
                        break;
                    case '\\':
                        result += '\\';
                        break;
                    case 'b':
                        result += '\b';
                        break;
                    case 'f':
                        result += '\f';
                        break;
                    case 'n':
                        result += '\n';
                        break;
                    case 'r':
                        result += '\r';
                        break;
                    case 't':
                        result += '\t';
                        break;
                        // Add additional cases for more escape sequences if needed
                    default:
                        result += '\\'; // If unrecognized escape sequence, keep the '\'
                        result += str[i + 1]; // Append the next character as is
                        break;
                    }
                    ++i; // Skip the next character as it was part of an escape sequence
                }
                else {
                    // If '\' is the last character, treat it as a literal backslash
                    result += '\\';
                }
            }
            else {
                // If it's a regular character, add it to the result
                result += str[i];
            }
        }

        return result;

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