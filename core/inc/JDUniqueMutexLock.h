#pragma once

#include "JD_base.h"
#include <mutex>

namespace JsonDatabase
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