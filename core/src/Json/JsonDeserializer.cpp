#include "Json/JsonDeserializer.h"

#ifdef JD_ENABLE_MULTITHREADING
#include <thread>
#include "utilities/AsyncContextDrivenDeleter.h"
#endif

namespace JsonDatabase
{

//#define DEBUG_PRINT(value) std::cout << "\n\n\n" << value << "\n\n\n";
#define DEBUG_PRINT(value)

    JsonValue JsonDeserializer::deserializeValue(const std::string& json) 
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
        std::string normalized;

        nornmalizeJsonString(json, normalized);
        JsonValue valOut;
        Buffer buff(normalized);
#ifdef JD_ENABLE_MULTITHREADING
        deserializeValueSplitted_internal(buff, valOut ,nullptr);
#else
        deserializeValue_internal(buff, valOut, nullptr);
#endif
        return valOut;
    }

    JsonValue JsonDeserializer::deserializeObject(const std::string& json) 
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
        std::string normalized;

        nornmalizeJsonString(json, normalized);
        JsonValue valOut;
        Buffer buff(normalized);
#ifdef JD_ENABLE_MULTITHREADING
        deserializeValueSplitted_internal(buff, valOut, nullptr);
#else
        deserializeValue_internal(buff, valOut, nullptr);
#endif
        return valOut;
    }

    JsonValue JsonDeserializer::deserializeArray(const std::string& json) 
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
        std::string normalized;

        nornmalizeJsonString(json, normalized);
        JsonValue valOut;
        Buffer buff(normalized);
#ifdef JD_ENABLE_MULTITHREADING
        deserializeArraySplitted_internal(buff, valOut, nullptr);
#else
        deserializeArray_internal(buff, valOut, nullptr);
#endif
        return valOut;
    }
    JsonValue JsonDeserializer::deserializeValue(const std::string& json, Internal::WorkProgress* progress)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
        std::string normalized;

        nornmalizeJsonString(json, normalized);
        JsonValue valOut;
        Buffer buff(normalized);
#ifdef JD_ENABLE_MULTITHREADING
        deserializeValueSplitted_internal(buff, valOut, progress);
#else
        deserializeValue_internal(buff, valOut, progress);
#endif
        if (progress)
            progress->setProgress(1);
        return valOut;
    }
    JsonValue JsonDeserializer::deserializeObject(const std::string& json, Internal::WorkProgress* progress)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
        std::string normalized;

        nornmalizeJsonString(json, normalized);
        JsonValue valOut;
        Buffer buff(normalized);
#ifdef JD_ENABLE_MULTITHREADING
        deserializeValueSplitted_internal(buff, valOut, progress);
#else
        deserializeValue_internal(buff, valOut, progress);
#endif
        return valOut;
    }
    JsonValue JsonDeserializer::deserializeArray(const std::string& json, Internal::WorkProgress* progress)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
        std::string normalized;

        nornmalizeJsonString(json, normalized);
        JsonValue valOut;
        Buffer buff(normalized);
#ifdef JD_ENABLE_MULTITHREADING
        deserializeArraySplitted_internal(buff, valOut, progress);
#else
        deserializeArray_internal(buff, valOut, progress);
#endif
        return valOut;
    }
    void JsonDeserializer::deserializeValue(const std::string& json, JsonValue& valueOut)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
        std::string normalized;

        nornmalizeJsonString(json, normalized);
        Buffer buff(normalized);
#ifdef JD_ENABLE_MULTITHREADING
        deserializeValueSplitted_internal(buff, valueOut, nullptr);
#else
        deserializeValue_internal(buff, valueOut, nullptr);
#endif
    }
    void JsonDeserializer::deserializeObject(const std::string& json, JsonValue& valueOut)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
        std::string normalized;

        nornmalizeJsonString(json, normalized);
        Buffer buff(normalized);
#ifdef JD_ENABLE_MULTITHREADING
        deserializeValueSplitted_internal(buff, valueOut, nullptr);
#else
        deserializeValue_internal(buff, valueOut, nullptr);
#endif
    }
    void JsonDeserializer::deserializeArray(const std::string& json, JsonValue& valueOut)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
        std::string normalized;

        nornmalizeJsonString(json, normalized);
        Buffer buff(normalized);
#ifdef JD_ENABLE_MULTITHREADING
        deserializeArraySplitted_internal(buff, valueOut, nullptr);
#else
        deserializeArray_internal(buff, valueOut, nullptr);
#endif
    }
    void JsonDeserializer::deserializeValue(const std::string& json, JsonValue& valueOut, Internal::WorkProgress* progress)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
        std::string normalized;

        nornmalizeJsonString(json, normalized);
        Buffer buff(normalized);
#ifdef JD_ENABLE_MULTITHREADING
        deserializeValueSplitted_internal(buff, valueOut, progress);
#else
        deserializeValue_internal(buff, valueOut, progress);
#endif
        if (progress)
            progress->setProgress(1);
    }
    void JsonDeserializer::deserializeObject(const std::string& json, JsonValue& valueOut, Internal::WorkProgress* progress)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
        std::string normalized;

        nornmalizeJsonString(json, normalized);
        Buffer buff(normalized);
#ifdef JD_ENABLE_MULTITHREADING
        deserializeValueSplitted_internal(buff, valueOut, progress);
#else
        deserializeValue_internal(buff, valueOut, progress);
#endif
    }
    void JsonDeserializer::deserializeArray(const std::string& json, JsonValue& valueOut, Internal::WorkProgress* progress)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
        std::string normalized;

        nornmalizeJsonString(json, normalized);
        Buffer buff(normalized);
#ifdef JD_ENABLE_MULTITHREADING
        deserializeArraySplitted_internal(buff, valueOut, progress);
#else
        deserializeArray_internal(buff, valueOut, progress);
#endif
    }

    void JsonDeserializer::deserializeValue_internal(Buffer& json, JsonValue& valOut, Internal::WorkProgress* progress)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
        switch (*json.current) {
        case '{':
        {
            deserializeObject_internal(json, valOut, progress);
            return;
        }
        case '[':
        {
            deserializeArray_internal(json, valOut, progress);
            return;
        }
        case '"':
        {
            valOut = JsonValue(std::string());
            std::string& str = std::get<std::string>(valOut.getVariant());
            deserializeString(json, str);
            return;
        }
        case 't':
        case 'f':
        {
            valOut = false;
            bool& value = std::get<bool>(valOut.getVariant());
            value = deserializeBool(json);
            return;
        }
        case 'n':
        {
            json.current += 4; // Skip the "null" keyword
            valOut = std::move(JsonValue());
            return;
        }
        default:
        {
            int intValue = 0;
            double doubleValue = 0;

            int result = deserializeNumber(json, intValue, doubleValue);
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
    void JsonDeserializer::deserializeValueSplitted_internal(Buffer& json, JsonValue& valOut, Internal::WorkProgress* progress)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
        switch (*json.current) {
            case '[':
            {
                deserializeArraySplitted_internal(json, valOut, progress);
                return;
            }        
            default:
                deserializeValue_internal(json, valOut, progress);
        }
    }
    void JsonDeserializer::deserializeObject_internal(Buffer& json, JsonValue& valOut, Internal::WorkProgress* progress)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_2);

        valOut = JsonObject();
        JsonObject& object = std::get<JsonObject>(valOut.getVariant());

        const char*& current = json.current;
        ++current; // Skip the '{' character
        if (*current != '}')
        {
            while (true)
            {
                if (*current == '}')
                    break;
                
                std::pair<std::string, JsonValue> pair;
                deserializePair(json, pair, progress);
                DEBUG_PRINT(value);
                object.emplace(std::move(pair));
                ++current; // Move to the next character or skip ',' or '}'
                //if (*current == '}')
                //    break;

            }
        }
        else
            ++current; // Skip the '}' character
        if (progress)
        {
            double progressValue = (double)(current - json.start) / (double)json.size;
            progress->setProgress(progressValue);
        }
    }
    void JsonDeserializer::deserializeArray_internal(Buffer& json, JsonValue& valOut, Internal::WorkProgress* progress)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
        valOut = JsonArray();
        JsonArray& array = std::get<JsonArray>(valOut.getVariant());
        array.reserve(100000);

        const char*& current = json.current;
        //current++; // Skip the '[' character
        ++current;
        if (*current != ']')
        {
            while (true)
            {
                if (*current == ']')
                    break;
                JsonValue value;
                deserializeValue_internal(json, value, progress);
                DEBUG_PRINT(value);
                array.emplace_back(std::move(value));
                ++current; // Skip the ']' character
                //if (*json.current == ']')
                //    break;
                //json.current++; // Move to the next character or skip ',' or ']'
            }
        }
        else
            ++current; // Skip the ']' character
        if(progress)
        {
            double progressValue = (double)(current - json.start) / (double)json.size;
            progress->setProgress(progressValue);
        }
    }
    void JsonDeserializer::deserializeArraySplitted_internal(Buffer& json, JsonValue& valOut, Internal::WorkProgress* progress)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
        valOut = JsonArray();
       // JsonArray& array = std::get<JsonArray>(valOut.getVariant());
       // array.reserve(100000);
        json.current++; // Skip the '[' character

        std::vector<ArrayObjectRange> rangeList;
        rangeList.reserve(1000);
        // Get individual object ranges
        findArrayObjectRange(json, rangeList);

        
#ifdef JD_ENABLE_MULTITHREADING
        unsigned int threadCount = std::thread::hardware_concurrency();
        if (threadCount > 100)
            threadCount = 100;
        size_t objCount = rangeList.size();
        if (threadCount > objCount/10)
            threadCount = objCount/10;

        if (threadCount > 1)
        {
            struct ThreadData
            {
                JsonArray array;
                size_t start; // Index in rangeList
                size_t end;
                Buffer data;
               // std::string data;
                std::atomic<unsigned long> processedCharCount;
            };

            JD_JSON_PROFILING_BLOCK("Starting threads", JD_COLOR_STAGE_3);
            std::vector<std::thread*> threads(threadCount, nullptr);
            std::vector<ThreadData*> threadData(threadCount, nullptr);
            size_t chunkSize = objCount / threadCount;
            size_t remainder = objCount % threadCount;
            size_t start = 0;
            size_t startIndex = json.current - json.start;
            for (size_t i = 0; i < threadCount; ++i)
            {
                ThreadData * data = new ThreadData();
                data->start = start;
                data->end = start + chunkSize;
                data->processedCharCount = 0;
                
                start += chunkSize;
                if (i == threadCount - 1)
                    data->end += remainder;

               // data->data = std::move(std::string(json.begin() + rangeList[data->start].start, json.begin() + rangeList[data->end-1].end+1));
                data->data.current = json.start + rangeList[data->start].start;
                data->data.start = data->data.current;
                data->data.size = rangeList[data->end - 1].end - rangeList[data->start].start + 1;
                data->data.end = data->data.start + data->data.size;


                threadData[i] = data;
                threads[i] = new std::thread([data, &rangeList, i]()
                    {
#if defined(JD_JSON_PROFILING_ENABED) && defined(JD_PROFILING)
                        std::string threadName = "Deserializer thread: " + std::to_string(i);
                        JD_JSON_PROFILING_THREAD(threadName.c_str());
#endif
                        std::atomic<unsigned long>& processedCharCount = data->processedCharCount;
                        size_t objCount = data->end - data->start - 1;
                        data->array.reserve(objCount);
                        size_t rangeStart = 0;
                        for (size_t j = data->start; j < data->end; ++j)
                        {
                            const ArrayObjectRange& objectRange = rangeList[j];
                            JsonValue value;
                            rangeStart = objectRange.start;
                            
                            deserializeObject_internal(data->data, value, nullptr);
                            data->data.current++; // skip the ',' character
                            data->array.emplace_back(std::move(value));
                            processedCharCount += objectRange.end - objectRange.start;

                            // ---------------------------------------------

                            /*
                            //const ArrayObjectRange &objectRange = rangeList[j];
                            //size_t rangeStart = objectRange.sta
                            
                            JsonValue value;
                            //deserializeObject_internal(json, rangeStart, value, nullptr);
                           // size_t indexDummy = rangeStart;
                            deserializeObject_internal(data->data, rangeStart, value, nullptr);
                            data->array.emplace_back(std::move(value));
                            //rangeStart += objectRange.end - objectRange.start;
                            ++rangeStart;
                            processedCharCount = rangeStart;*/
                        }
                    });
            }

            Internal::AsyncContextDrivenDeleter deleter(threadData);

            // Create a progress updater Thread
            std::thread* progressUpdater = nullptr;
            std::atomic<bool> progressUpdaterRunning = false;
            size_t endPos = rangeList.back().end;
            if (progress)
            {
                progressUpdaterRunning = true;
                
                progressUpdater = new std::thread([startIndex, &json, &threadData, endPos, progress, &progressUpdaterRunning, objCount]()
                    {
                        double divided = 1 / (double)endPos;
                        while (progressUpdaterRunning.load())
                        {
                            size_t finishCount = startIndex;
                            for (size_t i = 0; i < threadData.size(); ++i)
                            {
                                finishCount += threadData[i]->processedCharCount;
                            }
                            double progressValue = (double)finishCount * divided;
                            if(progressValue > 1)
								progressValue = 1;
                            progress->setProgress(progressValue);
                            if (finishCount < endPos / 2) // Only sleep if it is not almost finished
                                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                        }
                    });
            }
            JD_JSON_PROFILING_END_BLOCK;

            JD_JSON_PROFILING_BLOCK("Wait for threads", JD_COLOR_STAGE_3);
            // Wait for all threads to finish
            for (size_t i = 0; i < threadCount; ++i)
            {
                threads[i]->join();
                delete threads[i];
            }
            if (progressUpdater)
            {
                progressUpdaterRunning = false;
                progressUpdater->join();
                delete progressUpdater;
                progressUpdater = nullptr;
            }
            JD_JSON_PROFILING_END_BLOCK;

            JD_JSON_PROFILING_BLOCK("Combine parsed objects", JD_COLOR_STAGE_3);
            JsonArray& array = std::get<JsonArray>(valOut.getVariant());
            array.reserve(objCount);
            for (size_t i = 0; i < threadData.size(); ++i)
            {
                ThreadData *data = threadData[i];
				array.insert(array.end(), 
                    std::make_move_iterator(data->array.begin()), 
                    std::make_move_iterator(data->array.end()));
            }

            json.current = json.start + endPos + 1;
            JD_JSON_PROFILING_END_BLOCK;

            if (progress)
            {
                double progressValue = (double)(json.current - json.start) / (double)json.size;
                progress->setProgress(progressValue);
            }
        }
        else
#endif
        {
            json.current--;
            deserializeArray_internal(json, valOut, progress);
        }
    }
    void JsonDeserializer::deserializePair(Buffer& json, std::pair<std::string, JsonValue>& pairOut, Internal::WorkProgress* progress)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
        deserializeString(json, pairOut.first);
        json.current++; // Skip the colon ':'
        deserializeValue_internal(json, pairOut.second, progress);
    }
    void JsonDeserializer::deserializeString(Buffer& json, std::string& strOut)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_4);
        const char*& current = json.current;
        current++; // Skip the opening double quote
        size_t start = current - json.start;
        bool isString = true;
        bool lastCharWasNotEscape = true;
        while (isString)
        {
            const char &currentChar = *current;
            bool currentCharIsStringKey = currentChar == '"' && lastCharWasNotEscape;

            if (currentCharIsStringKey)
            {
                isString = false;
                break;
            }
            current++;

            lastCharWasNotEscape = currentChar != '\\';
        }
        

        strOut = std::move(std::string(json.start + start, current));
        current++; // Skip the closing double quote
    }
    
    int JsonDeserializer::deserializeNumber(const std::string& jsonString, int& intValue, double& doubleValue)
    {
        Buffer buff(jsonString);
        return deserializeNumber(buff, intValue, doubleValue);
    }
    int JsonDeserializer::deserializeNumber(Buffer& json, int& intValue, double& doubleValue)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_4);
        intValue = 0;
        doubleValue = 0;
       // std::size_t found = std::find_first_not_of("-0123456789.eE", index);
        const char* found = findFirstNotOfNumberStr(json.current);
        std::string subStr;
        if (found != nullptr)
        {
            subStr = std::move(std::string(json.current, found));
            json.current = found;
        }
        else
        {
            subStr = std::move(std::string(json.current, json.end));
            json.current++;
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

    bool JsonDeserializer::deserializeBool(Buffer& json)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_4);
        std::string str;
        if (*json.current == 't') {
            json.current += 4;
            return true;
        }
        else {
            json.current += 5;
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
    const char* JsonDeserializer::findFirstNotOfNumberStr(const char* str)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
        // return first char that is not "-0123456789.eE"
        for(const char *c = str; *c != '\0'; ++c)
		{
			switch (*c)
			{
				case '-':
				case '+':
				case 'e':
				case 'E':
				case '.':
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5': 
				case '6':
				case '7':
				case '8':
				case '9':
					break;
				default:
					return c;
			}
		}
		return nullptr;
    }




    void JsonDeserializer::findArrayObjectRange(Buffer& json, std::vector<ArrayObjectRange>& rangeList)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
        int contextCount = 0;
        ArrayObjectRange currentRange; 
        for(const char *c = json.current; c < json.end; ++c)
		{
            switch (*c)
            {
                case '{':
                {
                    if(contextCount == 0)
					{
						currentRange.start = c - json.start;
					}
                    contextCount++;
                    break;
                }
                case '}':
				{
					contextCount--;
                    if (contextCount == 0)
					{
						currentRange.end = c - json.start;
						rangeList.push_back(currentRange);
					}
					break;
				}
            }
		}
    }
}