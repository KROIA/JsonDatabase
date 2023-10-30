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

// Debugging
#ifdef NDEBUG
	#define JD_CONSOLE(msg)
	#define JD_CONSOLE_FUNCTION(msg)
#else
	#include <iostream>

	#define JD_DEBUG
	#define JD_CONSOLE_STREAM std::cout

	#define JD_CONSOLE(msg) JD_CONSOLE_STREAM << msg;
	#define JD_CONSOLE_FUNCTION(msg) JD_CONSOLE_STREAM << __PRETTY_FUNCTION__ << " " << msg;
#endif


#ifdef JD_PROFILING
#include "easy/profiler.h"

// Change the base color for the profiler. base color can be found in profiler::colors from profiler_clolors.h
#define JD_PROFILING_BASE_COLOR Cyan

#define BUILD_WITH_EASY_PROFILER
// Special expantion tecniques are required to combine the color name
#define CONCAT_SYMBOLS_IMPL(x, y) x##y
#define CONCAT_SYMBOLS(x, y) CONCAT_SYMBOLS_IMPL(x, y)
#define JD_PROFILING_COLOR(stage) profiler::colors:: CONCAT_SYMBOLS(JD_PROFILING_BASE_COLOR , stage)

// Different color stages
#define COLOR_STAGE_1 50
#define COLOR_STAGE_2 100
#define COLOR_STAGE_3 200
#define COLOR_STAGE_4 300
#define COLOR_STAGE_5 400
#define COLOR_STAGE_6 500
#define COLOR_STAGE_7 600
#define COLOR_STAGE_8 700
#define COLOR_STAGE_9 800
#define COLOR_STAGE_10 900
#define COLOR_STAGE_11 A100 
#define COLOR_STAGE_12 A200 
#define COLOR_STAGE_13 A400 
#define COLOR_STAGE_14 A700 

#define JD_PROFILING_BLOCK_C(text, colorStage) EASY_BLOCK(text, colorStage)
#define JD_PROFILING_FUNCTION_C(colorStage) EASY_FUNCTION(colorStage)
#define JD_PROFILING_BLOCK(text, colorStage) JD_PROFILING_BLOCK_C(text, JD_PROFILING_COLOR(colorStage))
#define JD_PROFILING_FUNCTION(colorStage) JD_PROFILING_FUNCTION_C(JD_PROFILING_COLOR(colorStage))


#else
#define JD_PROFILING_BLOCK_C(text, colorStage)
#define JD_PROFILING_FUNCTION_C(colorStage)
#define JD_PROFILING_BLOCK(text, color)
#define JD_PROFILING_FUNCTION(color)
#endif