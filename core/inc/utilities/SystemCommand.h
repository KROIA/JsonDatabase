#pragma once

#include "JsonDatabase_base.h"
#include "Logger.h"

namespace JsonDatabase
{
    namespace Internal
    {
        namespace SystemCommand
        {
            extern int execute(const std::string& command, Log::Logger::ContextLogger* logger = nullptr);
            extern std::string executePiped(const std::string& command, Log::Logger::ContextLogger *logger = nullptr);
        }
    }
}