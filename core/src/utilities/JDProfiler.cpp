#include "utilities/JDProfiler.h"

namespace JsonDatabase
{
    void Profiler::startProfiler()
    {
#ifdef JD_PROFILING
        EASY_PROFILER_ENABLE;
#endif
    }
    void Profiler::stopProfiler(const std::string &profileFilePath)
    {
#ifdef JD_PROFILING
        profiler::dumpBlocksToFile(profileFilePath.c_str());
#elif JD_ACTIVE_JSON == JD_JSON_GLAZE || JD_ACTIVE_JSON == JD_JSON_INTERNAL
        JD_UNUSED(profileFilePath);
#endif
    }
}