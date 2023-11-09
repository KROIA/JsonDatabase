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

#define USE_MUTEX_LOCK


#ifdef USE_MUTEX_LOCK
#define JDM_UNIQUE_LOCK std::unique_lock<std::mutex>(m_mutex);
#define JDM_UNIQUE_LOCK_P JDUniqueMutexLock uniqueLock(m_mutex);
#define JDM_UNIQUE_LOCK_M(MUT) std::unique_lock<std::mutex>(MUT);
#define JDM_UNIQUE_LOCK_P_M(mutex) JDUniqueMutexLock uniqueLock(mutex);
#else
#define JDM_UNIQUE_LOCK
#define JDM_UNIQUE_LOCK_P
#define JDM_UNIQUE_LOCK_M
#define JDM_UNIQUE_LOCK_P_M
#endif