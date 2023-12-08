#include "Json/JsonSerializer.h"

#if JD_ACTIVE_JSON == JD_JSON_INTERNAL

#include <iomanip> // For std::setprecision
#include <limits> // For std::numeric_limits
#include <omp.h>

#ifdef JD_ENABLE_MULTITHREADING
#include <thread>
#endif

#elif JD_ACTIVE_JSON == JD_JSON_GLAZE

#endif

namespace JsonDatabase
{
#if JD_ACTIVE_JSON == JD_JSON_INTERNAL

void JsonSerializer::enableTabs(bool enable)
{
    m_useTabs = enable;
}
void JsonSerializer::setTabSize(int size)
{
    m_tabSize = size;
}
void JsonSerializer::enableNewLinesInObjects(bool enable)
{
    m_useNewLinesInObjects = enable;
}
void JsonSerializer::enableNewLineAfterObject(bool enable)
{
    m_useNewLineAfterObject = enable;
}
void JsonSerializer::enableSpaces(bool enable)
{
    m_useSpaces = enable;
}
void JsonSerializer::setIndentChar(char indentChar)
{
    m_indentChar = indentChar;
}

bool JsonSerializer::tabsEnabled() const
{
    return m_useTabs;
}
int  JsonSerializer::tabSize() const
{
    return m_tabSize;
}
bool JsonSerializer::newLinesInObjectsEnabled() const
{
    return m_useNewLinesInObjects;
}
bool JsonSerializer::newLineAfterObjectEnabled() const
{
    return m_useNewLineAfterObject;
}
bool JsonSerializer::spacesEnabled() const
{
    return m_useSpaces;
}
char JsonSerializer::indentChar() const
{
    return m_indentChar;
}

std::string JsonSerializer::serializeValue(const JsonValue& value)
{
    JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
    std::string out;
    int indent = 0;
    serializeValue(value, out, indent);
    return out;
}
void JsonSerializer::serializeValue(const JsonValue& value, std::string& serializedOut)
{
    JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
    int indent = 0;
    serializeValue(value, serializedOut, indent);
}
void JsonSerializer::serializeValue(const JsonValue& value, std::string& serializedOut, int& indent)
{
    JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
    switch (value.m_type)
    {
    case JsonValue::Type::Null:
        serializedOut = serializeNull(); break;
    case JsonValue::Type::String:
        serializeString(std::get<std::string>(value.m_value), serializedOut); break;
    case JsonValue::Type::Int:
        serializeInt(std::get<int>(value.m_value), serializedOut); break;
    case JsonValue::Type::Double:
        serializeDouble(std::get<double>(value.m_value), serializedOut); break;
    case JsonValue::Type::Bool:
        serializeBool(std::get<bool>(value.m_value), serializedOut); break;
    case JsonValue::Type::Array:
        serializeArray(std::get<JsonArray>(value.m_value), serializedOut, indent); break;
    case JsonValue::Type::Object:
        serializeObject(*value.m_objElement, serializedOut, indent); break;
    }
}

std::string JsonSerializer::serializeObject(const JsonObject& object)
{
    JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
    std::string out;
    serializeObject(object, out);
    return out;
}
void JsonSerializer::serializeObject(const JsonObject& object, std::string& serializedOut)
{
    //JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
    int indent = 0;
    serializeObject(object, serializedOut, indent);
}
void JsonSerializer::serializeObject(const JsonObject& object, std::string& serializedOut, int& indent)
{
    JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
    JD_JSON_PROFILING_BLOCK("set up", JD_COLOR_STAGE_3);
    serializedOut = "{";
    std::string spaced(m_useSpaces, ' ');
    std::string tmp1;
    if (m_useNewLinesInObjects)
    {
        serializedOut += "\n";
        if (m_useTabs)
            indent += m_tabSize;

        tmp1 = std::string(indent, m_indentChar) + "\"";
    }
    else
        tmp1 = "\"";
    std::string tmp2 = "\":" + spaced;
    bool first = true;

    JD_JSON_PROFILING_END_BLOCK;
    JD_JSON_PROFILING_BLOCK("for loop", JD_COLOR_STAGE_3);
    for (const auto& pair : object) {
        if (!first) {
            serializedOut += ",";
            if (m_useNewLinesInObjects)
                serializedOut += "\n";
        }
        first = false;
        std::string value;
        serializeValue(pair.second, value, indent);
        std::string toAdd = tmp1 + std::string(pair.first) + tmp2 + std::move(value);
        serializedOut += std::move(toAdd);
    }
    JD_JSON_PROFILING_END_BLOCK;
    JD_JSON_PROFILING_BLOCK("finalize", JD_COLOR_STAGE_3);
    if (m_useNewLinesInObjects)
    {
        serializedOut += "\n";
        if (m_useTabs)
            indent -= m_tabSize;
    }
    serializedOut += std::move(std::string(indent, m_indentChar)) + "}";
    JD_JSON_PROFILING_END_BLOCK;
}


std::string JsonSerializer::serializeArray(const JsonArray& array)
{
    return serializeArray(array, nullptr);
}
std::string JsonSerializer::serializeArray(const JsonArray& array, Internal::WorkProgress* progress)
{
    //JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
    std::string out;
    int ident = 0;
    serializeArray(array, out, ident, progress);
    return out;
}
void JsonSerializer::serializeArray(const JsonArray& array, std::string& serializedOut)
{
    int indent = 0;
    serializeArray(array, serializedOut, indent);
}

void JsonSerializer::serializeArray(const JsonArray& array, std::string& serializedOut, int& indent)
{
    serializeArray(array, serializedOut, indent, nullptr);
}

void JsonSerializer::serializeArray(const JsonArray& array, std::string& serializedOut, int& indent, Internal::WorkProgress* progress)
{
    JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
    serializedOut = "[";
    if (m_useNewLineAfterObject)
    {
        serializedOut += "\n";
        if (m_useTabs)
            indent += m_tabSize;
    }
    const std::string indented(indent, m_indentChar);
    size_t sumSize = 0;
    std::vector<std::string> serializedValues;
    serializedValues.resize(array.size());
    double deltaProgress = 1.0 / (double)array.size();
#ifdef JD_ENABLE_MULTITHREADING
    unsigned int threadCount = std::thread::hardware_concurrency();
    if (threadCount > 100)
        threadCount = 100;
    if (array.size() > 100 && threadCount)
    {
        struct ThreadData
        {
            size_t start;
            size_t end;
            size_t sizeSum;
            int indent;
            std::atomic<int> finishCount;
        };
        std::vector<ThreadData> threadData(threadCount);
        std::vector<std::thread*> threads(threadCount, nullptr);
        size_t chunkSize = array.size() / threadCount;
        size_t remainder = array.size() % threadCount;
        size_t start = 0;
        for (size_t i = 0; i < threadCount; ++i)
        {
            threadData[i].start = start;
            threadData[i].end = start + chunkSize;
            threadData[i].sizeSum = 0;
            threadData[i].indent = indent;
            threadData[i].finishCount = 0;
            start += chunkSize;
            if (i == threadCount - 1)
                threadData[i].end += remainder;

            threads[i] = new std::thread([&threadData, &array, i, this, &indented, &serializedValues]()
                {
                    ThreadData& data = threadData[i];
                    std::atomic<int>& finishCount = threadData[i].finishCount;
                    for (size_t j = data.start; j < data.end; ++j)
                    {
                        const auto& value = array[j];
                        std::string valueText;
                        valueText.reserve(100);
                        std::string serializedValue;
                        serializeValue(value, serializedValue, data.indent);
                        valueText += ",";
                        if (m_useNewLineAfterObject)
                            valueText += "\n";
                        valueText += indented + std::move(serializedValue);
                        data.sizeSum += valueText.size();
                        serializedValues[j] = std::move(valueText);
                        finishCount++;
                    }
                    if (i == 0)
                    {
                        serializedValues[0].erase(serializedValues[0].begin());
                    }
                });
        }

        // Create a progress updater Thread
        std::thread* progressUpdater = nullptr;
        std::atomic<bool> progressUpdaterRunning = false;
        if (progress)
        {
            progressUpdaterRunning = true;
            size_t objectCount = array.size();
            progressUpdater = new std::thread([&threadData, progress, deltaProgress, &progressUpdaterRunning, objectCount]()
                {
                    while (progressUpdaterRunning.load())
                    {
                        int finishCount = 0;
                        for (size_t i = 0; i < threadData.size(); ++i)
                        {
                            finishCount += threadData[i].finishCount;
                        }

                        progress->setProgress((double)finishCount * deltaProgress);
                        if (finishCount < objectCount / 2) // Only sleep if it is not almost finished
                            std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    }
                });
        }

        for (size_t i = 0; i < threadCount; ++i)
        {
            threads[i]->join();
            sumSize += threadData[i].sizeSum;
            delete threads[i];
        }
        if (progressUpdater)
        {
            progressUpdaterRunning = false;
            progressUpdater->join();
            delete progressUpdater;
            progressUpdater = nullptr;
        }
    }
    else
#endif
    {

        bool first = true;
        for (size_t i = 0; i < array.size(); ++i)
        {
            const auto& value = array[i];
            std::string& valueText = serializedValues[i];
            valueText.reserve(100);

            if (!first) {
                valueText += ",";
                if (m_useNewLineAfterObject)
                    valueText += "\n";
            }
            first = false;

            std::string serializedValue;
            serializeValue(value, serializedValue);
            valueText += indented + std::move(serializedValue);
            sumSize += valueText.size();

            if (progress)
                progress->addProgress(deltaProgress);
        }
    }

    serializedOut.reserve(serializedOut.size() + sumSize + 50);
    for (auto& line : serializedValues)
    {
        serializedOut += std::move(line);
    }


    if (m_useNewLineAfterObject)
    {
        serializedOut += "\n";
        if (m_useTabs)
            indent -= m_tabSize;
    }
    serializedOut += std::move(std::string(indent, m_indentChar)) + "]";
}

std::string JsonSerializer::serializeString(const std::string& str)
{
    JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
    std::string out;
    escapeString(str, out);
    return out;
}
void JsonSerializer::serializeString(const std::string& str, std::string& serializedOut)
{
    JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
    escapeString(str, serializedOut);
}
std::string JsonSerializer::serializeInt(int value)
{
    JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
    return std::to_string(value);
}
void JsonSerializer::serializeInt(int value, std::string& serializedOut)
{
    JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
    serializedOut = std::move(std::to_string(value));
}
std::string JsonSerializer::serializeDouble(double value)
{
    JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
    std::string out;
    serializeDouble(value, out);
    return out;
}
void JsonSerializer::serializeDouble(double value, std::string& serializedOut)
{
    JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
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

    serializedOut = oss.str();
    size_t dotPos = serializedOut.find('.');
    if (dotPos != std::string::npos) {
        // Find position of last non-zero digit after the dot
        size_t lastNonZero = serializedOut.find_last_not_of('0');
        if (lastNonZero != std::string::npos)
        {
            if (serializedOut[lastNonZero] == '.')
                serializedOut.resize(lastNonZero + 2); // Truncate extra zeros after the dot
            else
                serializedOut.resize(lastNonZero + 1); // Truncate extra zeros after the dot
        }
    }
    else {
        serializedOut += ".0"; // Add a decimal point and a trailing zero if there's no decimal part
    }
}
std::string JsonSerializer::serializeBool(bool value)
{
    JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
    return value ? "true" : "false";
}
void JsonSerializer::serializeBool(bool value, std::string& serializedOut)
{
    JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
    if (value)
    {
        const std::string trueStr = "true";
        serializedOut = trueStr;
    }
    else
    {
        const std::string falseStr = "false";
        serializedOut = falseStr;
    }
}
const std::string& JsonSerializer::serializeNull()
{
    static const std::string null = "null";
    return null;
}
void JsonSerializer::escapeString(const std::string& str, std::string& serializedOut)
{
    JD_JSON_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
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
    serializedOut.resize(size + 2);
    serializedOut[0] = '"';
    serializedOut[size + 1] = '"';
    size_t pos = 1;
    for (char c : str)
    {
        switch (c)
        {
        case '"':
            serializedOut[pos++] = '\\';
            serializedOut[pos++] = '"';
            break;
        case '\\':
            serializedOut[pos++] = '\\';
            serializedOut[pos++] = '\\';
            break;
        case '\b':
            serializedOut[pos++] = '\\';
            serializedOut[pos++] = 'b';
            break;
        case '\f':
            serializedOut[pos++] = '\\';
            serializedOut[pos++] = 'f';
            break;
        case '\n':
            serializedOut[pos++] = '\\';
            serializedOut[pos++] = 'n';
            break;
        case '\r':
            serializedOut[pos++] = '\\';
            serializedOut[pos++] = 'r';
            break;
        case '\t':
            serializedOut[pos++] = '\\';
            serializedOut[pos++] = 't';
            break;
        default:
            serializedOut[pos++] = c;
            break;
        }
    }
}
#elif JD_ACTIVE_JSON == JD_JSON_GLAZE


    void JsonSerializer::enableTabs(bool enable)
    {
        JD_UNUSED(enable);
    }
    void JsonSerializer::setTabSize(int size)
    {
        JD_UNUSED(size);
    }
    void JsonSerializer::enableNewLinesInObjects(bool enable)
    {
        JD_UNUSED(enable);
    }
    void JsonSerializer::enableNewLineAfterObject(bool enable)
    {
        JD_UNUSED(enable);
    }
    void JsonSerializer::enableSpaces(bool enable)
    {
        JD_UNUSED(enable);
    }
    void JsonSerializer::setIndentChar(char indentChar)
    {
        JD_UNUSED(indentChar);
    }

    bool JsonSerializer::tabsEnabled() const
    {
        return false;
    }
    int  JsonSerializer::tabSize() const
    {
        return false;
    }
    bool JsonSerializer::newLinesInObjectsEnabled() const
    {
        return false;
    }
    bool JsonSerializer::newLineAfterObjectEnabled() const
    {
        return false;
    }
    bool JsonSerializer::spacesEnabled() const
    {
        return false;
    }
    char JsonSerializer::indentChar() const
    {
        return ' ';
    }




    std::string JsonSerializer::serializeValue(const JsonValue& value)
    {
        std::string buffer;
        glz::write < glz::opts{ .prettify = true } > (value, buffer);
        return std::move(buffer);
    }
    std::string JsonSerializer::serializeObject(const JsonObject& object)
    {
        std::string buffer;
        glz::write < glz::opts{ .prettify = true } > (object, buffer);
        return std::move(buffer);
    }
    std::string JsonSerializer::serializeArray(const JsonArray& array)
    {
        std::string buffer;
        glz::write < glz::opts{ .prettify = true } > (array, buffer);
        return std::move(buffer);
    }
    std::string JsonSerializer::serializeArray(const JsonArray& array, Internal::WorkProgress* progress)
    {
        std::string buffer;
        glz::write < glz::opts{ .prettify = true } > (array, buffer);
        if (progress)
            progress->setProgress(1.0);
        return std::move(buffer);
    }

    void JsonSerializer::serializeValue(const JsonValue& value, std::string& serializedOut)
    {
        glz::write < glz::opts{ .prettify = true } > (value, serializedOut);
    }
    void JsonSerializer::serializeObject(const JsonObject& object, std::string& serializedOut)
    {
        glz::write < glz::opts{ .prettify = true } > (object, serializedOut);
    }
    void JsonSerializer::serializeArray(const JsonArray& array, std::string& serializedOut)
    {
        glz::write < glz::opts{ .prettify = true } > (array, serializedOut);
    }



    std::string JsonSerializer::serializeInt(int value)
    {
        std::string buffer;
        glz::write < glz::opts{ .prettify = true } > (value, buffer);
        return std::move(buffer);
    }
    std::string JsonSerializer::serializeDouble(double value)
    {
        std::string buffer;
        glz::write < glz::opts{ .prettify = true } > (value, buffer);
        return std::move(buffer);
    }
    std::string JsonSerializer::serializeBool(bool value)
    {
        std::string buffer;
        glz::write < glz::opts{ .prettify = true } > (value, buffer);
        return std::move(buffer);
    }

    void JsonSerializer::serializeInt(int value, std::string& serializedOut)
    {
        glz::write < glz::opts{ .prettify = true } > (value, serializedOut);
    }
    void JsonSerializer::serializeDouble(double value, std::string& serializedOut)
    {
        glz::write < glz::opts{ .prettify = true } > (value, serializedOut);
    }
    void JsonSerializer::serializeBool(bool value, std::string& serializedOut)
    {
        glz::write < glz::opts{ .prettify = true } > (value, serializedOut);
    }

#endif
}