#pragma once
#include "JsonDatabase_base.h"
#include <string>

namespace JsonDatabase
{
	class JSON_DATABASE_EXPORT Profiler
	{
		public:
		static void startProfiler();
		static void stopProfiler(const std::string &profileFilePath);
	};
}
