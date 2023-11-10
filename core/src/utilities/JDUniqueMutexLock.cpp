#include "utilities/JDUniqueMutexLock.h"

namespace JsonDatabase
{
	JDUniqueMutexLock::JDUniqueMutexLock(std::mutex& mutex)
		: m_mutex(mutex)
	{
		JD_MUTEX_PROFILING_NONSCOPED_BLOCK("Mutex try get lock", JD_COLOR_STAGE_8);
		m_mutex.lock();
		JD_MUTEX_PROFILING_END_BLOCK;
		JD_MUTEX_PROFILING_NONSCOPED_BLOCK("Mutex locked", JD_COLOR_STAGE_10);
	}
	JDUniqueMutexLock::~JDUniqueMutexLock()
	{
		m_mutex.unlock();
		JD_MUTEX_PROFILING_END_BLOCK;
	}
}