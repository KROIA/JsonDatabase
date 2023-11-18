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
#define JDM_UNIQUE_LOCK_P JDUniqueMutexLock uniqueLock(m_mutex);
#define JDM_UNIQUE_LOCK_M(MUT) std::unique_lock<std::mutex> lck(MUT);
#define JDM_UNIQUE_LOCK_P_M(mutex) JDUniqueMutexLock uniqueLock(mutex);

//#define JD_USE_QJSON
#define JD_ENABLE_MULTITHREADING
