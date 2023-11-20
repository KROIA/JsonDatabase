#pragma once

#include <QtCore/qglobal.h>
#include <chrono>

#ifndef BUILD_STATIC
# if defined(JSONDATABASE_LIB)
#  define JSONDATABASE_EXPORT Q_DECL_EXPORT
# else
#  define JSONDATABASE_EXPORT Q_DECL_IMPORT
# endif
#else
# define JSONDATABASE_EXPORT
#endif

// MSVC Compiler
#ifdef _MSC_VER 
#define __PRETTY_FUNCTION__ __FUNCSIG__
typedef std::chrono::steady_clock::time_point TimePoint;
#else
typedef std::chrono::system_clock::time_point TimePoint;
#endif




#define JDM_UNIQUE_LOCK std::unique_lock<std::mutex> lck(m_mutex);
#define JDM_UNIQUE_LOCK_P Internal::JDUniqueMutexLock uniqueLock(m_mutex);
#define JDM_UNIQUE_LOCK_M(MUT) std::unique_lock<std::mutex> lck(MUT);
#define JDM_UNIQUE_LOCK_P_M(mutex) Internal::JDUniqueMutexLock uniqueLock(mutex);

/*
	Using the QT default Json library to parse objects.
	If commented out, the Builtin by this library Json structure will be used.
	QJson is slower.
*/
//#define JD_USE_QJSON

/*
	Enable multithreading for work that can be done in parallel.
	Disabling this will make the library not single threaded.
	The JsonManager still uses a thread to process the work

*/
#define JD_ENABLE_MULTITHREADING
