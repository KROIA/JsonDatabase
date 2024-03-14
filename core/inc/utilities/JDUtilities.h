#pragma once

#include "JsonDatabase_base.h"
#include "StringUtilities.h"
#include <string>
#include <windows.h>


namespace JsonDatabase
{
	namespace Utilities
	{
		std::string JSON_DATABASE_EXPORT_EXPORT getLastErrorString(DWORD error);

		

	}
}