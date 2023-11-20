#pragma once

#include "JD_base.h"
#include <mutex>

namespace JsonDatabase
{
	namespace Internal
	{
		class JSONDATABASE_EXPORT JDUniqueMutexLock
		{
		public:
			JDUniqueMutexLock(std::mutex& mutex);
			~JDUniqueMutexLock();
		private:
			std::mutex& m_mutex;
		};
	}
}