#pragma once

#include "JsonDatabase_base.h"

namespace JsonDatabase
{
    namespace Internal
    {
        namespace SystemCommand
        {
            extern int execute(const std::string& command);
            extern std::string executePiped(const std::string& command);
        }
    }
}