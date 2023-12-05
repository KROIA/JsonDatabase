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
#else
        JD_UNUSED(profileFilePath);
#endif
    }
}