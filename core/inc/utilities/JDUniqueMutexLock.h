#pragma once

#include "JsonDatabase_base.h"
#include <mutex>

namespace JsonDatabase
{
	namespace Internal
	{
		class JSON_DATABASE_EXPORT_EXPORT JDUniqueMutexLock
		{
		public:
			JDUniqueMutexLock(std::mutex& mutex);
			~JDUniqueMutexLock();
		private:
			std::mutex& m_mutex;
		};
	}
}