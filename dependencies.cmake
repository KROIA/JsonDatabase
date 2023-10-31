cmake_minimum_required(VERSION 3.1.0)



# External dependencies
include(FetchContent)
set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/dependencies")
set(RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/${RELATIVE_BUILD_FOLDER}")
set(LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/${RELATIVE_BUILD_FOLDER}")
set(ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/${RELATIVE_BUILD_FOLDER}")
set(DEBUG_POSTFIX "-d")

function(use_easy_profiler)
	#  EasyProfiler
	set(EASY_GIT_REPO  https://github.com/yse/easy_profiler.git)
	set(EASY_GIT_TAG   v2.1.0)


	FetchContent_Declare(
	 EASY_PROFILER
	 GIT_REPOSITORY ${EASY_GIT_REPO}
	 GIT_TAG ${EASY_GIT_TAG}
	)
	message("Download dependency: ${EASY_GIT_REPO}")

	set(EASY_PROFILER_NO_SAMPLES True)
	set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build easy_profiler as static library.")
	FetchContent_MakeAvailable(EASY_PROFILER)


	#set(EASY_PROFILER_NO_GUI False)
	set_target_properties(easy_profiler PROPERTIES CMAKE_RUNTIME_OUTPUT_DIRECTORY ${RUNTIME_OUTPUT_DIRECTORY})
	set_target_properties(easy_profiler PROPERTIES CMAKE_LIBRARY_OUTPUT_DIRECTORY ${RUNTIME_OUTPUT_DIRECTORY})
	set_target_properties(easy_profiler PROPERTIES CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${RUNTIME_OUTPUT_DIRECTORY})
	set_target_properties(easy_profiler PROPERTIES DEBUG_POSTFIX ${DEBUG_POSTFIX})
	target_compile_definitions(easy_profiler PUBLIC EASY_PROFILER_STATIC)
endfunction()
# End external dependencies