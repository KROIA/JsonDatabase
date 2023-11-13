#pragma once

#include "JD_base.h"
#include <string>
#include <windows.h>

namespace JsonDatabase
{
	namespace Utilities
	{
		extern std::string getLastErrorString(DWORD error);
	}
}