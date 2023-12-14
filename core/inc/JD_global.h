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
#define JD_JSON_QT 1
#define JD_JSON_INTERNAL 2


#define JD_ACTIVE_JSON JD_JSON_INTERNAL
/*
	Enable multithreading for work that can be done in parallel.
	Disabling this will make the library not single threaded.
	The JsonManager still uses a thread to process the work

*/
#define JD_ENABLE_MULTITHREADING

#ifdef JD_ENABLE_MULTITHREADING
#define JD_ENABLE_MULTITHREADING_JSON_PARSER
#endif

#define JD_UNUSED(x) (void)x;

#if defined(JSONDATABASE_LIB)
#pragma warning (error : 4715) // not all control paths return a value shuld be an error instead of a warning
#pragma warning (error : 4700) // uninitialized local variable used shuld be an error instead of a warning
#pragma warning (error : 4244) // Implicit conversions between data types 
#pragma warning (error : 4100) // Unused variables
#pragma warning (error : 4018) // Type mismatch 
#pragma warning (error : 4996) // Unsafe function calls
#pragma warning (error : 4456) // declaration of 'x' hides previous local declaration
#pragma warning (error : 4065) // switch statement contains 'default' but no 'case' labels
#pragma warning (error : 4189) // Unused return value
#pragma warning (error : 4996) // unsafe function calls
#pragma warning (error : 4018) // signed/unsigned mismatch
#endif
