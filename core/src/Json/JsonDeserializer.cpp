#include "Json/JsonDeserializer.h"

namespace JsonDatabase
{

//#define DEBUG_PRINT(value) std::cout << "\n\n\n" << value << "\n\n\n";
#define DEBUG_PRINT(value)

    JsonValue JsonDeserializer::deserializeValue(const std::string& json) 
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
        std::string normalized;
        size_t index = 0;
        nornmalizeJsonString(json, normalized);
        JsonValue valOut;
        deserializeValue_internal(normalized, index, valOut ,nullptr);
        return valOut;
    }

    JsonValue JsonDeserializer::deserializeObject(const std::string& json) 
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
        std::string jsonString;
        size_t index = 0;
        nornmalizeJsonString(json, jsonString);
        JsonValue valOut;
        deserializeObject_internal(jsonString, index, valOut, nullptr);
        return valOut;
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
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
        std::string jsonString;
        size_t index = 0;
        nornmalizeJsonString(json, jsonString);
        JsonValue valOut;
        deserializeArray_internal(jsonString, index, valOut, nullptr);
        return valOut;
        /*JsonArray array;
        size_t index = 1; // Skip the '[' character
        while (jsonString[index] != ']') {
            auto value = deserializeValue_internal(jsonString, index);
            array.push_back(value);
            index++; // Move to the next character or skip ',' or ']'
        }
        return JsonValue(array);*/
    }
    JsonValue JsonDeserializer::deserializeValue(const std::string& json, Internal::WorkProgress* progress)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
        std::string normalized;
        size_t index = 0;
        nornmalizeJsonString(json, normalized);
        JsonValue valOut;
        deserializeValue_internal(normalized, index, valOut, progress);
        if (progress)
            progress->setProgress(1);
        return valOut;
    }
    JsonValue JsonDeserializer::deserializeObject(const std::string& json, Internal::WorkProgress* progress)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
        std::string jsonString;
        size_t index = 0;
        nornmalizeJsonString(json, jsonString);
        JsonValue valOut;
        deserializeObject_internal(jsonString, index, valOut, progress);
        return valOut;
    }
    JsonValue JsonDeserializer::deserializeArray(const std::string& json, Internal::WorkProgress* progress)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
        std::string jsonString;
        size_t index = 0;
        nornmalizeJsonString(json, jsonString);
        JsonValue valOut;
	    deserializeArray_internal(jsonString, index, valOut, progress);
        return valOut;
    }
    void JsonDeserializer::deserializeValue(const std::string& json, JsonValue& valueOut)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
        std::string normalized;
        size_t index = 0;
        nornmalizeJsonString(json, normalized);
        deserializeValue_internal(normalized, index, valueOut, nullptr);
    }
    void JsonDeserializer::deserializeObject(const std::string& json, JsonValue& valueOut)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
        std::string jsonString;
        size_t index = 0;
        nornmalizeJsonString(json, jsonString);
        deserializeObject_internal(jsonString, index, valueOut, nullptr);
    }
    void JsonDeserializer::deserializeArray(const std::string& json, JsonValue& valueOut)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
        std::string jsonString;
        size_t index = 0;
        nornmalizeJsonString(json, jsonString);
        deserializeArray_internal(jsonString, index, valueOut, nullptr);
    }
    void JsonDeserializer::deserializeValue(const std::string& json, JsonValue& valueOut, Internal::WorkProgress* progress)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
        std::string normalized;
        size_t index = 0;
        nornmalizeJsonString(json, normalized);
        deserializeValue_internal(normalized, index, valueOut, progress);
        if (progress)
            progress->setProgress(1);
    }
    void JsonDeserializer::deserializeObject(const std::string& json, JsonValue& valueOut, Internal::WorkProgress* progress)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
        std::string jsonString;
        size_t index = 0;
        nornmalizeJsonString(json, jsonString);
        deserializeObject_internal(jsonString, index, valueOut, progress);
    }
    void JsonDeserializer::deserializeArray(const std::string& json, JsonValue& valueOut, Internal::WorkProgress* progress)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
        std::string jsonString;
        size_t index = 0;
        nornmalizeJsonString(json, jsonString);
        deserializeArray_internal(jsonString, index, valueOut, progress);
    }

    /*void JsonDeserializer::deserializeValue_internal(const std::string& json, size_t& index, JsonValue& valOut)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
        switch (json[index]) {
            case '{':
            {
                deserializeObject_internal(json, index, valOut);
                return;
            }
            case '[':
            {
                deserializeArray_internal(json, index, valOut);
                return;
            }
            case '"':
            {
                valOut = JsonValue(std::string());
                std::string &str = std::get<std::string>(valOut.getVariant());
                deserializeString(json, index, str);
                return;
            }
            case 't':
            case 'f':
            {
                valOut = false;
                bool& value = std::get<bool>(valOut.getVariant());
                value = deserializeBool(json, index);
                return;
            }
            case 'n':
            {
                index += 4; // Skip the "null" keyword
                valOut = std::move(JsonValue());
                return;
            }
            default:
            {
                int intValue = 0;
                double doubleValue = 0;

                int result = deserializeNumber(json, intValue, doubleValue, index);
                if (result == 1)
                {
                    valOut = intValue;
                    return;
                }

                valOut = doubleValue;
                return;
            }
        }
    }

    void JsonDeserializer::deserializeObject_internal(const std::string& json, size_t& index, JsonValue &valOut)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_2);

        valOut = JsonObject();
        JsonObject &object = std::get<JsonObject>(valOut.getVariant());
        index++; // Skip the '{' character
        //skipWhiteSpace(json, index);
        while (json[index] != '}') {
            std::pair<std::string, JsonValue> pair;
            deserializePair(json, index, pair);
            DEBUG_PRINT(value);
            object.insert(std::move(pair));
            if (json[index] == '}')
                break;
            index++; // Move to the next character or skip ',' or '}'
        }
        index++; // Skip the '}' character
    }

    void JsonDeserializer::deserializeArray_internal(const std::string& json, size_t& index, JsonValue &valOut)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
        valOut = JsonArray();
        JsonArray &array = std::get<JsonArray>(valOut.getVariant());
        array.reserve(100000);
        index++; // Skip the '[' character
        while (json[index] != ']') {
            JsonValue value;
            deserializeValue_internal(json, index, value);
            DEBUG_PRINT(value);
            array.emplace_back(value);
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
    }*/
    void JsonDeserializer::deserializeValue_internal(const std::string& json, size_t& index, JsonValue& valOut, Internal::WorkProgress* progress)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
        switch (json[index]) {
        case '{':
        {
            deserializeObject_internal(json, index, valOut, progress);
            return;
        }
        case '[':
        {
            deserializeArray_internal(json, index, valOut, progress);
            return;
        }
        case '"':
        {
            valOut = JsonValue(std::string());
            std::string& str = std::get<std::string>(valOut.getVariant());
            deserializeString(json, index, str);
            return;
        }
        case 't':
        case 'f':
        {
            valOut = false;
            bool& value = std::get<bool>(valOut.getVariant());
            value = deserializeBool(json, index);
            return;
        }
        case 'n':
        {
            index += 4; // Skip the "null" keyword
            valOut = std::move(JsonValue());
            return;
        }
        default:
        {
            int intValue = 0;
            double doubleValue = 0;

            int result = deserializeNumber(json, intValue, doubleValue, index);
            if (result == 1)
            {
                valOut = intValue;
                return;
            }

            valOut = doubleValue;
            return;
        }
        }
    }
    void JsonDeserializer::deserializeObject_internal(const std::string& json, size_t& index, JsonValue& valOut, Internal::WorkProgress* progress)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_2);

        valOut = JsonObject();
        JsonObject& object = std::get<JsonObject>(valOut.getVariant());
        index++; // Skip the '{' character
        //skipWhiteSpace(json, index);
        while (json[index] != '}') {
            std::pair<std::string, JsonValue> pair;
            deserializePair(json, index, pair, progress);
            //DEBUG_PRINT(value);
           // object.insert(std::move(pair));
            object.emplace(std::move(pair));
            if (json[index] == '}')
                break;
            index++; // Move to the next character or skip ',' or '}'
            //if (progress)
            //   progress->setProgress((double)index / (double)json.size());
        }
        //valOut = std::move(object);
        index++; // Skip the '}' character
        if(progress)
            progress->setProgress((double)index / (double)json.size());
    }
    void JsonDeserializer::deserializeArray_internal(const std::string& json, size_t& index, JsonValue& valOut, Internal::WorkProgress* progress)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
        valOut = JsonArray();
        JsonArray& array = std::get<JsonArray>(valOut.getVariant());
        array.reserve(100000);
        index++; // Skip the '[' character

        while (json[index] != ']') {
            JsonValue value;
            deserializeValue_internal(json, index, value, progress);
            

            DEBUG_PRINT(value);
            array.emplace_back(std::move(value));
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
        if(progress)
            progress->setProgress((double)index / (double)json.size());
        /* {
            JD_JSON_PROFILING_BLOCK("move array", JD_COLOR_STAGE_3);
            valOut = std::move(array);
        }*/
    }

    /*void JsonDeserializer::deserializePair(const std::string& json, size_t& index, std::pair<std::string, JsonValue>& pairOut)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
        deserializeString(json, index, pairOut.first);
        index++; // Skip the colon ':'
        deserializeValue_internal(json, index, pairOut.second);
    }*/
    void JsonDeserializer::deserializePair(const std::string& json, size_t& index, std::pair<std::string, JsonValue>& pairOut, Internal::WorkProgress* progress)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
        deserializeString(json, index, pairOut.first);
        index++; // Skip the colon ':'
        deserializeValue_internal(json, index, pairOut.second, progress);
    }
    void JsonDeserializer::deserializeString(const std::string& json, size_t& index, std::string& strOut)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_4);
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
        

        strOut = std::move(std::string(json.begin() + start, json.begin() + index));
        //strOut = json.substr(start, index - start);
        index++; // Skip the closing double quote
    }
    /*void JsonDeserializer::deserializeString(const std::string& json, size_t& index, std::string& strOut, Internal::WorkProgress* progress)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_4);
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


        strOut = std::move(std::string(json.begin() + start, json.begin() + index));
        index++; // Skip the closing double quote
        if (progress)
            progress->setProgress((double)index / (double)json.size());
    }*/


    int JsonDeserializer::deserializeNumber(const std::string& str, int& intValue, double& doubleValue, size_t& index)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_4);
        intValue = 0;
        doubleValue = 0;
        std::size_t found = str.find_first_not_of("-0123456789.eE", index);
        std::string subStr;
        if (found != std::string::npos)
        {
            subStr = str.substr(index, found - index);
            index += found - index;
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

    bool JsonDeserializer::deserializeBool(const std::string& json, size_t& index) 
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_4);
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

    void JsonDeserializer::skipWhiteSpace(const std::string& jsonString, size_t &index) 
    {
        while (index < jsonString.size() && std::isspace(jsonString[index])) {
            index++;
        }
    }
    void JsonDeserializer::nornmalizeJsonString(const std::string& jsonString, std::string& jsonStringOut)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
		//removeChars(jsonString, jsonStringOut, " \n\t\r");
        removeSpecificChars(jsonString, jsonStringOut);
    }
    void JsonDeserializer::removeChars(const std::string& jsonString, std::string& jsonStringOut,
        const std::string & removingChars)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
        jsonStringOut.resize(jsonString.size()+1); // Resize without initializing characters
        size_t count = 0;
        bool isString = false;
        bool lastCharWasNotEscape = true;

        for (auto currentChar : jsonString) // Test 2
        {
            bool currentCharIsStringKey = currentChar == '"' && lastCharWasNotEscape;

            if (currentCharIsStringKey)
            {
                isString = !isString;
            }

            // If in string, ignore the remove chars command
            if (!isString)
            {
                for (auto j : removingChars)
                {
                    if (currentChar == j)
                    {
                        goto skip;
                    }
                }
            }
            jsonStringOut[count++] = currentChar;
        skip:;
            lastCharWasNotEscape = currentChar != '\\';
        }
        jsonStringOut[count] = '\0';
    }
    void JsonDeserializer::removeSpecificChars(const std::string& jsonString, std::string& jsonStringOut)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
        jsonStringOut.resize(jsonString.size()+1); // Resize without initializing characters
        size_t count = 0;
        bool isString = false;
        bool lastCharWasNotEscape = true;
       
        for(auto currentChar : jsonString) 
        {
            bool currentCharIsStringKey = currentChar == '"' && lastCharWasNotEscape;
       
            if (currentCharIsStringKey)
            {
                isString = !isString;
            }
            
            // If in string, ignore the remove chars command
            if (!isString)
            {
                // Remove these characters from the string
                switch (currentChar)
                {
                case ' ':
                case '\n':
                case '\t':
                case '\r':
                    goto skip;
                }
            }
            jsonStringOut[count++] = currentChar;
            skip:;
            lastCharWasNotEscape = currentChar != '\\';
        }
        jsonStringOut[count] = '\0';
    }

    std::string JsonDeserializer::unescapeString(const std::string& str)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
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
}