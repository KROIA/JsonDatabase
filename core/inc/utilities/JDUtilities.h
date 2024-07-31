#pragma once

#include "JsonDatabase_base.h"
#include "StringUtilities.h"
#include <string>
#include <windows.h>
#include "Logger.h"


namespace JsonDatabase
{
	namespace Utilities
	{
		std::string JSON_DATABASE_EXPORT getLastErrorString(DWORD error);
		std::string JSON_DATABASE_EXPORT calculateMD5Hash(const std::string& filePath, Log::LogObject *logger, bool &success);
		

	}
}
