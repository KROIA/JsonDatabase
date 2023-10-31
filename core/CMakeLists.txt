cmake_minimum_required(VERSION 3.1.0)

set(JsonDatabase_filename "JsonDatabase")
set(JsonDatabase_Include "inc")


if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()



MACRO(FILE_DIRECTORIES return_list ending)
    FILE(GLOB_RECURSE new_list ${ending})
    SET(dir_list "")
    FOREACH(file_path ${new_list})
        GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)
        SET(dir_list ${dir_list} ${file_path})
    ENDFOREACH()
    LIST(REMOVE_DUPLICATES dir_list)
    SET(${return_list} ${dir_list})
ENDMACRO()




# QT settings
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
#set(CMAKE_AUTOUIC ON)

include(${QT_LOCATOR_CMAKE})
message("Search for QT5 finished")
find_package(Qt5Widgets REQUIRED)
message("find_package for Qt5Widgets finished")

if (Qt5Widgets_FOUND)
    message(STATUS "Using Qt v ${Qt5Widgets_VERSION}")
else()
    message(STATUS "INFO\n\n\tQt5 not found!\n")
endif()


# end QT settings

# External dependencies
include(FetchContent)
set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/dependencies")

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
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build easy_profiler not as shared library.")
FetchContent_MakeAvailable(EASY_PROFILER)

message(">> BUILD_SHARED_LIBS: ${BUILD_SHARED_LIBS}")

#set(EASY_PROFILER_NO_GUI False)
set_target_properties(easy_profiler PROPERTIES CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/${RELATIVE_BUILD_FOLDER}")
set_target_properties(easy_profiler PROPERTIES CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/${RELATIVE_BUILD_FOLDER}")
set_target_properties(easy_profiler PROPERTIES CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/${RELATIVE_BUILD_FOLDER}")
set_target_properties(easy_profiler PROPERTIES DEBUG_POSTFIX "-d")
target_compile_definitions(easy_profiler PUBLIC  EASY_PROFILER_STATIC)

# End external dependencies

include_directories(inc)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

FILE_DIRECTORIES(H_FILES *.h)
FILE_DIRECTORIES(CPP_FILES *.cpp)

qt5_wrap_cpp(CPP_MOC_FILES ${H_FILES})


set(SOURCES
    ${CPP_FILES}
    ${CPP_MOC_FILES}
    ${H_FILES}
)

message("CMAKE_CURRENT_BINARY_DIR = " ${CMAKE_CURRENT_BINARY_DIR})

message("Create library with files:")
 FOREACH(file ${SOURCES})
    message("  File: " ${file})
 ENDFOREACH()



set(JsonDatabase_dependencies 
   Qt5::Core)

add_library(JsonDatabase_shared SHARED ${SOURCES} )
add_library(JsonDatabase_static STATIC ${SOURCES} )
add_library(JsonDatabase_static_profile STATIC ${SOURCES})
 
target_compile_definitions(JsonDatabase_shared PRIVATE  JSONDATABASE_LIB)
target_compile_definitions(JsonDatabase_static PRIVATE  BUILD_STATIC JSONDATABASE_LIB)
target_compile_definitions(JsonDatabase_static_profile PRIVATE  BUILD_STATIC JD_PROFILING JSONDATABASE_LIB)


set_target_properties(JsonDatabase_shared PROPERTIES LINKER_LANGUAGE CXX  OUTPUT_NAME "${JsonDatabase_filename}" DEBUG_POSTFIX "-d")
set_target_properties(JsonDatabase_static PROPERTIES LINKER_LANGUAGE CXX  OUTPUT_NAME "${JsonDatabase_filename}-s" DEBUG_POSTFIX "-d")
set_target_properties(JsonDatabase_static_profile PROPERTIES LINKER_LANGUAGE CXX  OUTPUT_NAME "${JsonDatabase_filename}-s-p" DEBUG_POSTFIX "-d")


target_link_libraries(JsonDatabase_shared ${JsonDatabase_dependencies})
target_link_libraries(JsonDatabase_static ${JsonDatabase_dependencies})
target_link_libraries(JsonDatabase_static_profile ${JsonDatabase_dependencies} easy_profiler)


target_include_directories(JsonDatabase_shared PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${JsonDatabase_Include}>
    ${CMAKE_CURRENT_BINARY_DIR}
	# $<INSTALL_INTERFACE:inc>  # <prefix>/inc 
)
target_include_directories(JsonDatabase_static PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${JsonDatabase_Include}>
    ${CMAKE_CURRENT_BINARY_DIR}
	# $<INSTALL_INTERFACE:inc>  # <prefix>/inc 
)
target_include_directories(JsonDatabase_static_profile PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${JsonDatabase_Include}>
    ${CMAKE_CURRENT_BINARY_DIR}
	# $<INSTALL_INTERFACE:inc>  # <prefix>/inc 
)
# Define the install path
message("Installation to path: ${CMAKE_INSTALL_PREFIX}/bin")
message("Installation to path: ${INSTALL_PATH}")
install(TARGETS JsonDatabase_shared DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
install(TARGETS JsonDatabase_shared DESTINATION ${INSTALL_PATH})
install(TARGETS JsonDatabase_static DESTINATION ${INSTALL_PATH})
install(TARGETS JsonDatabase_static_profile DESTINATION ${INSTALL_PATH})
install(TARGETS easy_profiler DESTINATION ${INSTALL_PATH})


