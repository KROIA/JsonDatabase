#pragma once

#include "JD_base.h"
#include "StringUtilities.h"
#include <string>
#include <windows.h>


namespace JsonDatabase
{
	namespace Utilities
	{
		std::string JSONDATABASE_EXPORT getLastErrorString(DWORD error);

		

	}
}