#include "Json/JsonDeserializer.h"

#if JD_ACTIVE_JSON == JD_JSON_INTERNAL

#ifdef JD_ENABLE_MULTITHREADING_JSON_PARSER
#include <thread>
#include "utilities/AsyncContextDrivenDeleter.h"
#include <windows.h>
#endif

#elif JD_ACTIVE_JSON == JD_JSON_GLAZE

#endif

namespace JsonDatabase
{


#if JD_ACTIVE_JSON == JD_JSON_INTERNAL

    //#define DEBUG_PRINT(value) std::cout << "\n\n\n" << value << "\n\n\n";
#define DEBUG_PRINT(value)

    JsonValue JsonDeserializer::deserializeValue(const std::string& json)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
        JsonValue valOut;
        
        std::string normalized;
        nornmalizeJsonString(json, normalized);
        Buffer buff(normalized);
#ifdef JD_ENABLE_MULTITHREADING_JSON_PARSER
        deserializeValueSplitted_internal(buff, valOut);
#else
        deserializeValue_internal(buff, valOut);
#endif
        return valOut;
    }

    JsonObject JsonDeserializer::deserializeObject(const std::string& json)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
        std::string normalized;

        nornmalizeJsonString(json, normalized);
        JsonObject valOut;
        Buffer buff(normalized);
        deserializeObject_internal(buff, valOut);
        return valOut;
    }

    JsonArray JsonDeserializer::deserializeArray(const std::string& json)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
        
        JsonArray valOut;
        
        std::string normalized;
        nornmalizeJsonString(json, normalized);
        Buffer buff(normalized);
#ifdef JD_ENABLE_MULTITHREADING_JSON_PARSER
        deserializeArraySplitted_internal(buff, valOut, nullptr);
#else
        deserializeArray_internal(buff, valOut);
#endif
        return valOut;
    }
    JsonValue JsonDeserializer::deserializeValue(const std::string& json, Internal::WorkProgress* progress)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
        
        JsonValue valOut;
        
        std::string normalized;
        nornmalizeJsonString(json, normalized);
        Buffer buff(normalized);
#ifdef JD_ENABLE_MULTITHREADING_JSON_PARSER
        deserializeValueSplitted_internal(buff, valOut, progress);
#else
        deserializeValue_internal(buff, valOut, progress);
#endif
        progress->setProgress(1);
        return valOut;
    }
    JsonObject JsonDeserializer::deserializeObject(const std::string& json, Internal::WorkProgress* progress)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
        std::string normalized;

        nornmalizeJsonString(json, normalized);
        JsonObject valOut;
        Buffer buff(normalized);
        deserializeObject_internal(buff, valOut, progress);
        return valOut;
    }
    JsonArray JsonDeserializer::deserializeArray(const std::string& json, Internal::WorkProgress* progress)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
        
        JsonArray valOut;
        
        std::string normalized;
        nornmalizeJsonString(json, normalized);
        Buffer buff(normalized);
#ifdef JD_ENABLE_MULTITHREADING_JSON_PARSER
        deserializeArraySplitted_internal(buff, valOut, progress);
#else
        deserializeArray_internal(buff, valOut, progress);
#endif
        return valOut;
    }
    bool JsonDeserializer::deserializeValue(const std::string& json, JsonValue& valueOut)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
        
        std::string normalized;
        nornmalizeJsonString(json, normalized);
        Buffer buff(normalized);
#ifdef JD_ENABLE_MULTITHREADING_JSON_PARSER
        return deserializeValueSplitted_internal(buff, valueOut);
#else
        return deserializeValue_internal(buff, valueOut);
#endif
    }
    bool JsonDeserializer::deserializeObject(const std::string& json, JsonObject& valueOut)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
        std::string normalized;

        nornmalizeJsonString(json, normalized);
        Buffer buff(normalized);
        return deserializeObject_internal(buff, valueOut);
    }
    bool JsonDeserializer::deserializeArray(const std::string& json, JsonArray& valueOut)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
        
        std::string normalized;
        nornmalizeJsonString(json, normalized);
        Buffer buff(normalized);
#ifdef JD_ENABLE_MULTITHREADING_JSON_PARSER
        return deserializeArraySplitted_internal(buff, valueOut, nullptr);
#else
        return deserializeArray_internal(buff, valueOut);
#endif
    }
    bool JsonDeserializer::deserializeValue(const std::string& json, JsonValue& valueOut, Internal::WorkProgress* progress)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
        std::string normalized;
        nornmalizeJsonString(json, normalized);
        Buffer buff(normalized);
#ifdef JD_ENABLE_MULTITHREADING_JSON_PARSER
        return deserializeValueSplitted_internal(buff, valueOut, progress);
#else
        return deserializeValue_internal(buff, valueOut, progress);
#endif
        progress->setProgress(1);
    }
    bool JsonDeserializer::deserializeObject(const std::string& json, JsonObject& valueOut, Internal::WorkProgress* progress)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
        std::string normalized;

        nornmalizeJsonString(json, normalized);
        Buffer buff(normalized);
        return deserializeObject_internal(buff, valueOut, progress);
    }
    bool JsonDeserializer::deserializeArray(const std::string& json, JsonArray& valueOut, Internal::WorkProgress* progress)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
        std::string normalized;
        nornmalizeJsonString(json, normalized);
        Buffer buff(normalized);
#ifdef JD_ENABLE_MULTITHREADING_JSON_PARSER
        return deserializeArraySplitted_internal(buff, valueOut, progress);
#else
        return deserializeArray_internal(buff, valueOut, progress);
#endif
    }




    bool JsonDeserializer::deserializeValue_internal(Buffer& json, JsonValue& valOut)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
        switch (json.peek())
        {
            case '{':
            {
                std::shared_ptr<JsonObject> objPtr = std::make_shared<JsonObject>();
                if (deserializeObject_internal(json, *objPtr.get()))
                {
                    valOut = std::move(objPtr);
                    return true;
                }
                break;
            }
            case '[':
            {
                std::shared_ptr<JsonArray> arrPtr = std::make_shared<JsonArray>();
                arrPtr->reserve(100);
                if (deserializeArray_internal(json, *arrPtr.get()))
                {
                    valOut = std::move(arrPtr);
                    return true;
                }
                break;
            }
            case '"':
            {
                // valOut = JsonValue(std::move(std::string()));
                // std::string& str = std::get<std::string>(valOut.getVariant());
                // deserializeString(json, str);

                std::string str;
                str.reserve(100);
                if (deserializeString(json, str))
                {
                    valOut = std::move(str);
                    return true;
                }
                break;
            }
            case 't':
            case 'f':
            {
                bool value;
                if (deserializeBool(json, value))
                {
                    valOut = value;
                    return true;
                }
                break;
            }
            case 'n':
            {
                json.skip(4); // Skip the "null" keyword
                valOut = std::move(JsonValue());
                return json.getRemainingSize();
            }
            default:
            {
                int intValue = 0;
                double doubleValue = 0;

                int result = deserializeNumber(json, intValue, doubleValue);
                if (result == 1)
                {
                    valOut = intValue;
                    return json.getRemainingSize();;
                }

                valOut = doubleValue;
                return json.getRemainingSize();
            }
        }
        return false;
    }
    bool JsonDeserializer::deserializeValue_internal(Buffer& json, JsonValue& valOut, Internal::WorkProgress* progress)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
        switch (json.peek())
        {
            case '{':
            {
                std::shared_ptr<JsonObject> objPtr = std::make_shared<JsonObject>();
                if (deserializeObject_internal(json, *objPtr.get(), progress))
                {
                    valOut = std::move(objPtr);
                    return true;
                }
                break;
            }
            case '[':
            {
                std::shared_ptr<JsonArray> arrPtr = std::make_shared<JsonArray>();
                arrPtr->reserve(100);
                if (deserializeArray_internal(json, *arrPtr.get(), progress))
                {
                    valOut = std::move(arrPtr);
                    return true;
                }
                break;
            }
            case '"':
            {
                // valOut = JsonValue(std::move(std::string()));
                // std::string& str = std::get<std::string>(valOut.getVariant());
                // deserializeString(json, str);

                std::string str;
                str.reserve(100);
                if (deserializeString(json, str))
                {
                    valOut = std::move(str);
                    return true;
                }
                break;
            }
            case 't':
            case 'f':
            {
                
                bool value;
                if (deserializeBool(json, value))
                {
                    valOut = value;
                    return true;
                }
                break;
            }
            case 'n':
            {
                json.skip(4); // Skip the "null" keyword
                valOut = std::move(JsonValue());
                return json.getRemainingSize();
            }
            default:
            {
                int intValue = 0;
                double doubleValue = 0;

                int result = deserializeNumber(json, intValue, doubleValue);
                if (result == 1)
                {
                    valOut = intValue;
                    return json.getRemainingSize();
                }

                valOut = doubleValue;
                return json.getRemainingSize();
            }
        }
        return false;
    }





    bool JsonDeserializer::deserializeValueSplitted_internal(Buffer& json, JsonValue& valOut)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
        switch (json.peek())
        {
        case '[':
        {
            std::shared_ptr<JsonArray> arrPtr = std::make_shared<JsonArray>();
            if (deserializeArraySplitted_internal(json, *arrPtr.get(), nullptr))
            {
                valOut = std::move(arrPtr);
                return true;
            }
            return false;
        }
        default:
            return deserializeValue_internal(json, valOut);
        }
        return false;
    }
    bool JsonDeserializer::deserializeValueSplitted_internal(Buffer& json, JsonValue& valOut, Internal::WorkProgress* progress)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
        switch (json.peek())
        {
        case '[':
        {
            std::shared_ptr<JsonArray> arrPtr = std::make_shared<JsonArray>();
            if (deserializeArraySplitted_internal(json, *arrPtr.get(), progress))
            {
                valOut = std::move(arrPtr);
                return true;
            }
            return false;
        }
        default:
            return deserializeValue_internal(json, valOut, progress);
        }
        return false;
    }





    bool JsonDeserializer::deserializeObject_internal(Buffer& json, JsonObject& valOut)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
       // const char*& current = json.current;
        json.skip(); // Skip the '{' character
        if (json.peek() != '}')
        {
            while (true)
            {
                std::pair<std::string, JsonValue> pair;
                if (deserializePair(json, pair))
                {
                    DEBUG_PRINT(value);
                    valOut.emplace(std::move(pair));
                    if (json.peek() == '}')
                        break;
                    json.skip(); // Move to the next character or skip ',' or '}'
                }
                else
                    break;
            }
        }
        json.skip(); // Skip the '}' character
        return json.getRemainingSize();
    }
    bool JsonDeserializer::deserializeObject_internal(Buffer& json, JsonObject& valOut, Internal::WorkProgress* progress)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
        //const char*& current = json.current;
        json.skip(); // Skip the '{' character
        if (json.peek() != '}')
        {
            while (true)
            {
                std::pair<std::string, JsonValue> pair;
                if (deserializePair(json, pair, progress))
                {
                    DEBUG_PRINT(value);
                    valOut.emplace(std::move(pair));
                    if (json.peek() == '}')
                        break;
                    json.skip(); // Move to the next character or skip ',' or '}'
                }
                else
                    break;
            }
        }
        json.skip(); // Skip the '}' character
        double progressValue = (double)(json.getIndex()) / (double)json.size();
        progress->setProgress(progressValue);
        return json.getRemainingSize();
    }


    /*void JsonDeserializer::deserializeObjectSplitted_internal(Buffer& json, JsonObject& out)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
        switch (*json.current)
        {
        case '{':
        {
            JsonArray arr;
            deserializeArraySplitted_internal(json, arr, nullptr);
            valOut = std::move(arr);
            return;
        }
        default:
            deserializeValue_internal(json, valOut);
        }
    }
    void JsonDeserializer::deserializeObjectSplitted_internal(Buffer& json, JsonObject& out, Internal::WorkProgress* progress)
    {

    }*/




    bool JsonDeserializer::deserializeArray_internal(Buffer& json, JsonArray& valOut)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
        valOut.reserve(1000);

        //const char*& current = json.current;
        json.skip();
        if (json.peek() != ']')
        {
            while (true)
            {
                JsonValue value;
                if (deserializeValue_internal(json, value))
                {
                    DEBUG_PRINT(value);
                    valOut.emplace_back(std::move(value));
                    if (json.peek() == ']')
                        break;
                    json.skip(); // Skip the ']' character
                }
                else
                    break;
            }
        }
        json.skip(); // Skip the ']' character
        return json.getRemainingSize();
    }
    bool JsonDeserializer::deserializeArray_internal(Buffer& json, JsonArray& valOut, Internal::WorkProgress* progress)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
        valOut.reserve(1000);

       // const char*& current = json.current;
        json.skip();
        bool eof = false;
        if (json.peek() != ']')
        {
            while (true)
            {
                JsonValue value;
                if (deserializeValue_internal(json, value, progress))
                {
                    DEBUG_PRINT(value);
                    valOut.emplace_back(std::move(value));
                    if (json.peek() == ']')
                        break;
                    json.skip(); // Skip the ']' character
                }
                else
                    break;                
            }
        }
        json.skip(); // Skip the ']' character
        double progressValue = (double)(json.getIndex()) / (double)json.size();
        progress->setProgress(progressValue);
        return json.getRemainingSize();
    }
    bool JsonDeserializer::deserializeArraySplitted_internal(Buffer& json, JsonArray& valOut, Internal::WorkProgress* progress)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
        json.skip(); // Skip the '[' character

       /* std::string normalized;
        const std::string rawJson(json.start(), json.size());
        nornmalizeJsonString(rawJson, normalized);
        Buffer buff(normalized);
        buff.skip();*/
#ifdef JD_ENABLE_MULTITHREADING_JSON_PARSER

        

        std::vector<ArrayObjectRange> rangeList;
        rangeList.reserve(1000);
        // Get individual object ranges
        findArrayObjectRange(json, rangeList);
        //unsigned int threadCount = 2;// std::thread::hardware_concurrency();
        unsigned int threadCount = std::thread::hardware_concurrency();
        if (threadCount > 100)
            threadCount = 100;
        size_t objCount = rangeList.size();
        if (threadCount > objCount / 10)
            threadCount = objCount / 10;

        //constexpr size_t padding = 128;
        //constexpr size_t spacing = 128;
        //threadCount = 0;
        if (threadCount > 1)
        {
            //threadCount = 5;
            // threadCount = 2;
            


            struct ThreadData
            {
                JsonArray array;
                size_t start = 0; // Index in rangeList
                size_t end = 0;
                const char* rawInputData = nullptr;
                size_t rawInputDataSize = 0;

                Buffer data;
                char *dataStr = nullptr;
                std::atomic<unsigned long> processedCharCount;

                ~ThreadData()
				{
					delete[] dataStr;
				}
            };

            JD_JSON_PROFILING_BLOCK("Starting threads", JD_COLOR_STAGE_3);
            std::vector<std::thread*> threads(threadCount, nullptr);
            std::vector<ThreadData*> threadData(threadCount, nullptr);
            size_t chunkSize = objCount / threadCount;
            size_t remainder = objCount % threadCount;
            size_t start = 0;
            size_t startIndex = json.getIndex();
            for (size_t i = 0; i < threadCount; ++i)
            {
                size_t currentIndex =  i;
                ThreadData *data = new ThreadData;
                
                data->start = start;
                data->end = start + chunkSize;
                data->processedCharCount = 0;

                start += chunkSize;
                if (i == threadCount - 1)
                    data->end += remainder;

                data->rawInputData = json.start() + rangeList[data->start].start;
                data->rawInputDataSize = rangeList[data->end - 1].end - rangeList[data->start].start + 1;

                //data.dataStr = std::move(std::string(json.start() + rangeList[data.start].start, rangeList[data.end - 1].end - rangeList[data.start].start + 1));
                // data->data = std::move(std::string(json.begin() + rangeList[data->start].start, json.begin() + rangeList[data->end-1].end+1));
                
                //JD_JSON_PROFILING_BLOCK("Copy string", JD_COLOR_STAGE_4);
                //data->dataStr = new char[data->rawInputDataSize];
                //memcpy(data->dataStr, data->rawInputData, data->rawInputDataSize);
                //JD_JSON_PROFILING_END_BLOCK;
                //data->data.setString(data->dataStr, data->rawInputDataSize);


                //data->data.setString(json.start() + rangeList[data->start].start, size);
                //data->data.current = json.start + rangeList[data->start].start;
                //data->data.start = data->data.current;
                //data->data.size = rangeList[data->end - 1].end - rangeList[data->start].start + 1;
                //data->data.end = data->data.start + data->data.size;


                threadData[currentIndex] = data;
                std::thread* newThread = new std::thread([data, &rangeList, i]()
                    {
#if defined(JD_JSON_PROFILING_ENABED) && defined(JD_PROFILING)
                        std::string threadName = "Deserializer thread: " + std::to_string(i);
                        JD_JSON_PROFILING_THREAD(threadName.c_str());
#endif
                        
                        JD_JSON_PROFILING_BLOCK("Copy string", JD_COLOR_STAGE_4);
                        data->dataStr = new char[data->rawInputDataSize];
                       // removeSpecificChars(data->rawInputData, data->dataStr, data->rawInputDataSize);
                        memcpy(data->dataStr, data->rawInputData, data->rawInputDataSize);
                        data->data.setString(data->dataStr, data->rawInputDataSize);
                        JD_JSON_PROFILING_END_BLOCK;


                        std::atomic<unsigned long>& processedCharCount = data->processedCharCount;
                        size_t objCount = data->end - data->start + 5;
                        data->array.reserve(objCount);
                        size_t rangeStart = 0;

                        for (size_t j = data->start; j < data->end; ++j)
                        {
                            const ArrayObjectRange& objectRange = rangeList[j];
                            JsonObject value;
                            rangeStart = objectRange.start;

                            deserializeObject_internal(data->data, value);
                            data->data.skip(); // skip the ',' character
                            data->array.emplace_back(std::move(value));
                            processedCharCount += objectRange.end - objectRange.start;
                        }
                    });

                // Define on which cpu this thread is allowed to run
                DWORD_PTR dw = SetThreadAffinityMask(newThread->native_handle(), DWORD_PTR(1) << i);
                if (dw == 0)
                {
                    DWORD dwErr = GetLastError();
                    JD_CONSOLE_FUNCTION("SetThreadAffinityMask failed, GLE=" << dwErr << '\n');
                }
                threads[i] = newThread;
            }

            Internal::AsyncContextDrivenDeleter deleter(threadData);

            // Create a progress updater Thread
            std::thread* progressUpdater = nullptr;
            std::atomic<bool> progressUpdaterRunning = false;
            size_t endPos = rangeList.back().end;
            if (progress)
            {
                progressUpdaterRunning = true;

                progressUpdater = new std::thread([threadCount,startIndex, &json, &threadData, endPos, progress, &progressUpdaterRunning, objCount]()
                    {
                        double divided = 1 / (double)endPos;
                        while (progressUpdaterRunning.load())
                        {
                            size_t finishCount = startIndex;
                            for (size_t i = 0; i < threadCount; ++i)
                            {
                                finishCount += threadData[i]->processedCharCount;
                            }
                            double progressValue = (double)finishCount * divided;
                            if (progressValue > 1)
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
            valOut.reserve(objCount);
            for (size_t i = 0; i < threadCount; ++i)
            {
                ThreadData* data = threadData[i];
                valOut.insert(valOut.end(),
                    std::make_move_iterator(data->array.begin()),
                    std::make_move_iterator(data->array.end()));
            }

            json.setCurrent(json.start() + endPos + 1);
            JD_JSON_PROFILING_END_BLOCK;

            if (progress)
            {
                double progressValue = (double)(json.getIndex()) / (double)json.size();
                progress->setProgress(progressValue);
            }
        }
        else
#endif
        {
            json.skipBack();
            if (progress)
                return deserializeArray_internal(json, valOut, progress);
            else
                return deserializeArray_internal(json, valOut);
        }
        return false;
    }


    bool JsonDeserializer::deserializePair(Buffer& json, std::pair<std::string, JsonValue>& pairOut)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
        bool result = deserializeString(json, pairOut.first);
        if (result)
        {
            json.skip(); // Skip the colon ':'
            return deserializeValue_internal(json, pairOut.second);
        }
        return false;
    }
    bool JsonDeserializer::deserializePair(Buffer& json, std::pair<std::string, JsonValue>& pairOut, Internal::WorkProgress* progress)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
        bool success = deserializeString(json, pairOut.first);
        if (success)
        {
            json.skip(); // Skip the colon ':'
            return deserializeValue_internal(json, pairOut.second, progress);
        }
        return false;
    }
    bool JsonDeserializer::deserializeString(Buffer& json, std::string& strOut)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_4);
        //const char*& current = json.current;

        json.skip(); // Skip the opening double quote
        const char* startOfText = json.getCurrent();
        // size_t start = current - json.start;
        bool isString = true;
        //bool lastCharWasNotEscape = true;
        bool eof = false;
        while (isString)
        {
            // Find the end of the string (the closing double quote)
            int count = 0;
            while (json.peek() != '"' && !eof)
            {
                eof = !json.getRemainingSize();
                if (eof)
                {
                    return false;
                }
                json.skip();
            }
            // gfdjgfdgfd "Te\\"xt" fdsfes
            if (*(json.getCurrent() - 1) != '\\')
            {
                break;
            }

            // Search backwards to check if
            
            for (const char* c = json.getCurrent() - 1; c >= startOfText; --c)
            {
                if (*c == '\\')
                {
                    ++count;
                    continue;
                }
                if (count % 2 == 0)
                    break;
                goto skip;
            }
        skip:;
            json.skip();
        }
        std::string escapedString(startOfText, json.getCurrent());
        strOut = std::move(unescapeString(escapedString));
        json.skip();
        return json.getRemainingSize();
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
        const char* found = findFirstNotOfNumberStr(json.getCurrent());
        std::string subStr;
        if (found != nullptr)
        {
            subStr = std::move(std::string(json.getCurrent(), found));
            json.setCurrent(found);
        }
        else
        {
            subStr = std::move(std::string(json.getCurrent(), json.end()));
            json.skip();
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

    bool JsonDeserializer::deserializeBool(Buffer& json, bool& valueOut)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_4);
        std::string str;
        if (json.peek() == 't') {
            json.skip(4);
            valueOut = true;
        }
        else {
            json.skip(5);
            valueOut = true;
        }
        return json.getRemainingSize();
    }

    void JsonDeserializer::skipWhiteSpace(const std::string& jsonString, size_t& index)
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
        const std::string& removingChars)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
        jsonStringOut.resize(jsonString.size() + 1); // Resize without initializing characters
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
        jsonStringOut.resize(jsonString.size() + 1); // Resize without initializing characters
        size_t count = 0;
        bool isString = false;
        bool lastCharWasNotEscape = true;

        for (auto currentChar : jsonString)
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
    void JsonDeserializer::removeSpecificChars(const char* jsonString, char* jsonStringOut, size_t size)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
        size_t count = 0;
        bool isString = false;
        bool lastCharWasNotEscape = true;

        for(size_t i=0; i<size; ++i)
       // for (auto currentChar : jsonString)
        {
            const char &currentChar = jsonString[i];
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


    const char* JsonDeserializer::findFirstNotOfNumberStr(const char* str)
    {
        JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
        // return first char that is not "-0123456789.eE"
        for (const char* c = str; *c != '\0'; ++c)
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
        for (const char* c = json.getCurrent(); c < json.end(); ++c)
        {
            switch (*c)
            {
            case '{':
            {
                if (contextCount == 0)
                {
                    currentRange.start = c - json.start();
                }
                contextCount++;
                break;
            }
            case '}':
            {
                contextCount--;
                if (contextCount == 0)
                {
                    currentRange.end = c - json.start();
                    rangeList.push_back(currentRange);
                }
                break;
            }
            }
        }
    }

    

#elif JD_ACTIVE_JSON == JD_JSON_GLAZE
    JsonValue JsonDeserializer::deserializeValue(const std::string& json)
    {
        JsonValue value;
        //auto err = glz::read_json(value, json);
        //if(err.ec != glz::error_code::none)
		//{
        //    JD_CONSOLE_FUNCTION("Error while parsing json: " << (int)err.ec);
		//}
		return value;
    }
    JsonObject JsonDeserializer::deserializeObject(const std::string& json)
    {
        JsonObject value;
        //auto err = glz::read_json(value, json);
        //if (err.ec != glz::error_code::none)
        //{
        //    JD_CONSOLE_FUNCTION("Error while parsing json: " << (int)err.ec);
        //}
        return std::move(value);
    }
    JsonArray JsonDeserializer::deserializeArray(const std::string& json)
    {
        JsonArray value;
        //auto err = glz::read_json(value, json);
        //if (err.ec != glz::error_code::none)
        //{
        //    JD_CONSOLE_FUNCTION("Error while parsing json: " << (int)err.ec);
        //}
        return std::move(value);
    }
    JsonValue JsonDeserializer::deserializeValue(const std::string& json, Internal::WorkProgress* progress)
    {
        JsonValue value;
        //auto err = glz::read_json(value, json);
        //if (err.ec != glz::error_code::none)
        //{
        //    JD_CONSOLE_FUNCTION("Error while parsing json: " << (int)err.ec);
        //}
        //if (progress)
        //    progress->setProgress(1);
        return value;
    }
    JsonObject JsonDeserializer::deserializeObject(const std::string& json, Internal::WorkProgress* progress)
    {
        JsonObject value;
        //auto err = glz::read_json(value, json);
        //if (err.ec != glz::error_code::none)
        //{
        //    JD_CONSOLE_FUNCTION("Error while parsing json: " << (int)err.ec);
        //}
        //if (progress)
        //    progress->setProgress(1);
        return std::move(value);
    }
    JsonArray JsonDeserializer::deserializeArray(const std::string& json, Internal::WorkProgress* progress)
    {
        JsonArray value;
        //auto err = glz::read_json(value, json);
        //if (err.ec != glz::error_code::none)
        //{
        //    JD_CONSOLE_FUNCTION("Error while parsing json: " << (int)err.ec);
        //}
        //if(progress)
        //    progress->setProgress(1);
        return value;
    }

    void JsonDeserializer::deserializeValue(const std::string& json, JsonValue& valueOut)
    {
        //auto err = glz::read_json(valueOut, json);
        //if (err.ec != glz::error_code::none)
        //{
        //    JD_CONSOLE_FUNCTION("Error while parsing json: " << (int)err.ec);
        //}
    }
    void JsonDeserializer::deserializeObject(const std::string& json, JsonObject& valueOut)
    {
        //auto err = glz::read_json(valueOut, json);
        //if (err.ec != glz::error_code::none)
        //{
        //    JD_CONSOLE_FUNCTION("Error while parsing json: " << (int)err.ec);
        //}
    }
    void JsonDeserializer::deserializeArray(const std::string& json, JsonArray& valueOut)
    {
        //auto err = glz::read_json(valueOut, json);
        //if (err.ec != glz::error_code::none)
        //{
        //    JD_CONSOLE_FUNCTION("Error while parsing json: " << (int)err.ec);
        //}
    }
    void JsonDeserializer::deserializeValue(const std::string& json, JsonValue& valueOut, Internal::WorkProgress* progress)
    {
        //auto err = glz::read_json(valueOut, json);
        //if (err.ec != glz::error_code::none)
        //{
        //    JD_CONSOLE_FUNCTION("Error while parsing json: " << (int)err.ec);
        //}
        if (progress)
            progress->setProgress(1);
    }
    void JsonDeserializer::deserializeObject(const std::string& json, JsonObject& valueOut, Internal::WorkProgress* progress)
    {
        //auto err = glz::read_json(valueOut, json);
        //if (err.ec != glz::error_code::none)
        //{
        //    JD_CONSOLE_FUNCTION("Error while parsing json: " << (int)err.ec);
        //}
        if (progress)
            progress->setProgress(1);
    }
    void JsonDeserializer::deserializeArray(const std::string& json, JsonArray& valueOut, Internal::WorkProgress* progress)
    {
        //auto err = glz::read_json(valueOut, json);
        //if (err.ec != glz::error_code::none)
        //{
        //    JD_CONSOLE_FUNCTION("Error while parsing json: " << (int)err.ec);
        //}
        if (progress)
            progress->setProgress(1);
    }

    void JsonDeserializer::nornmalizeJsonString(const std::string& jsonString, std::string& jsonStringOut)
    {
        JD_UNUSED(jsonString);
        JD_UNUSED(jsonStringOut);
    }
    int JsonDeserializer::deserializeNumber(const std::string& jsonString, int& intValue, double& doubleValue)
    {
        JD_UNUSED(jsonString);
        JD_UNUSED(intValue);
        JD_UNUSED(doubleValue);
        return 0;
    }
#endif

#if JD_ACTIVE_JSON == JD_JSON_GLAZE || JD_ACTIVE_JSON == JD_JSON_INTERNAL
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
#endif

}