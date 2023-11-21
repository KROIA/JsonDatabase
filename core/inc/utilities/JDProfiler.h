#pragma once
#include "JD_base.h"
#include <string>

namespace JsonDatabase
{
	class JSONDATABASE_EXPORT Profiler
	{
		public:
		static void startProfiler();
		static void stopProfiler(const std::string &profileFilePath);
	};
}