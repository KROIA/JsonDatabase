﻿cmake_minimum_required(VERSION 3.15)

project(JsonDatabase)
set(PARALLEL_BUILD OFF)
#
# -----------------------------------------
#
# Check if variable is set, otherwise set to default
if(NOT RELATIVE_BUILD_FOLDER)
	set(RELATIVE_BUILD_FOLDER build)
endif()
if(NOT CMAKE_INSTALL_PREFIX)
	set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR})
endif()
if(NOT RELATIVE_INSTALL_FOLDER)
	set(RELATIVE_INSTALL_FOLDER lib)
endif()
 
# Set output paths
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/${RELATIVE_BUILD_FOLDER}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/${RELATIVE_BUILD_FOLDER}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/${RELATIVE_BUILD_FOLDER}")

link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
set(QT_LOCATOR_CMAKE "${CMAKE_CURRENT_SOURCE_DIR}/QtLocator.cmake")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(PARALLEL_BUILD)
	set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} /MP)
	set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} /MP)
endif()


add_subdirectory(core)
add_subdirectory(Examples)
