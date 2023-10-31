cmake_minimum_required(VERSION 3.1.0)

set(LIBRARY_NAME JsonDatabase)
set(LIB_DEFINE JSONDATABASE_LIB)
set(LIB_PROFILE_DEFINE JD_PROFILING)

set(INCLUDE_PATH "inc")

# Enable/disable QT modules for this example
set(QT_ENABLE ON)
# Find the necessary QT modules
set(QT_MODULES
    Core
    # Add any other required modules here
)


set(LIBRARY_NAME_STATIC ${LIBRARY_NAME}_static)
set(LIBRARY_NAME_SHARED ${LIBRARY_NAME}_shared)
set(LIBRARY_NAME_STATIC_PROFILE ${LIBRARY_NAME_STATIC}_profile)



if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()



# QT settings
if(QT_ENABLE)
    include(${QT_LOCATOR_CMAKE})
    find_package(Qt5Widgets REQUIRED)

    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    #set(CMAKE_AUTOUIC ON)

    find_package(Qt5 REQUIRED COMPONENTS ${QT_MODULES})
endif()
# end QT settings

#  # External dependencies
#  include(FetchContent)
#  set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/dependencies")
#  
#  #  EasyProfiler
#  set(EASY_GIT_REPO  https://github.com/yse/easy_profiler.git)
#  set(EASY_GIT_TAG   v2.1.0)
#  
#  
#  FetchContent_Declare(
#   EASY_PROFILER
#   GIT_REPOSITORY ${EASY_GIT_REPO}
#   GIT_TAG ${EASY_GIT_TAG}
#  )
#  message("Download dependency: ${EASY_GIT_REPO}")
#  
#  set(EASY_PROFILER_NO_SAMPLES True)
#  set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build easy_profiler not as shared library.")
#  FetchContent_MakeAvailable(EASY_PROFILER)
#  
#  
#  #set(EASY_PROFILER_NO_GUI False)
#  set_target_properties(easy_profiler PROPERTIES CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/${RELATIVE_BUILD_FOLDER}")
#  set_target_properties(easy_profiler PROPERTIES CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/${RELATIVE_BUILD_FOLDER}")
#  set_target_properties(easy_profiler PROPERTIES CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/${RELATIVE_BUILD_FOLDER}")
#  set_target_properties(easy_profiler PROPERTIES DEBUG_POSTFIX "-d")
#  target_compile_definitions(easy_profiler PUBLIC  EASY_PROFILER_STATIC)
#  
#  # End external dependencies
use_easy_profiler()

include_directories(inc)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

FILE_DIRECTORIES(H_FILES *.h)
FILE_DIRECTORIES(CPP_FILES *.cpp)

#qt5_wrap_cpp(CPP_MOC_FILES ${H_FILES})


set(SOURCES
    ${CPP_FILES}
    ${H_FILES}
)

if(QT_ENABLE)
    # Search for QT specific files
    FILE_DIRECTORIES(UI_FILES *.ui)    
    FILE_DIRECTORIES(RES_FILES *.qrc)    

    qt5_wrap_cpp(CPP_MOC_FILES ${H_FILES})
    qt5_wrap_ui(UIS_HDRS ${UI_FILES})
    qt5_add_resources(RESOURCE_FILES ${RES_FILES})

    set(SOURCES ${SOURCES}
	    ${CPP_MOC_FILES}
	    ${UIS_HDRS}
        ${RESOURCE_FILES})

    # Link the QT modules to your executable
    foreach(MODULE ${QT_MODULES})
        set(QT_LIBS ${QT_LIBS} Qt5::${MODULE})
    endforeach()

endif()

message("CMAKE_CURRENT_BINARY_DIR = " ${CMAKE_CURRENT_BINARY_DIR})

message("Create library with files:")
 FOREACH(file ${SOURCES})
    message("  File: " ${file})
 ENDFOREACH()



set(LIBRARY_DEPENDENCIES
   ${QT_LIBS})

add_library(${LIBRARY_NAME_SHARED}          SHARED ${SOURCES} )
add_library(${LIBRARY_NAME_STATIC}          STATIC ${SOURCES} )
add_library(${LIBRARY_NAME_STATIC_PROFILE}  STATIC ${SOURCES})
 
target_compile_definitions(${LIBRARY_NAME_SHARED} PRIVATE  ${LIB_DEFINE})
target_compile_definitions(${LIBRARY_NAME_STATIC} PRIVATE  BUILD_STATIC ${LIB_DEFINE})
target_compile_definitions(${LIBRARY_NAME_STATIC_PROFILE} PRIVATE  BUILD_STATIC ${${LIB_PROFILE_DEFINE}} ${LIB_DEFINE})


set_target_properties(${LIBRARY_NAME_SHARED} PROPERTIES LINKER_LANGUAGE CXX  OUTPUT_NAME "${LIBRARY_NAME}" DEBUG_POSTFIX "-d")
set_target_properties(${LIBRARY_NAME_STATIC} PROPERTIES LINKER_LANGUAGE CXX  OUTPUT_NAME "${LIBRARY_NAME}-s" DEBUG_POSTFIX "-d")
set_target_properties(${LIBRARY_NAME_STATIC_PROFILE} PROPERTIES LINKER_LANGUAGE CXX  OUTPUT_NAME "${LIBRARY_NAME}-s-p" DEBUG_POSTFIX "-d")


target_link_libraries(${LIBRARY_NAME_SHARED} ${LIBRARY_DEPENDENCIES})
target_link_libraries(${LIBRARY_NAME_STATIC}  ${LIBRARY_DEPENDENCIES})
target_link_libraries(${LIBRARY_NAME_STATIC_PROFILE} ${LIBRARY_DEPENDENCIES} easy_profiler)


target_include_directories(${LIBRARY_NAME_SHARED} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${INCLUDE_PATH}>
    ${CMAKE_CURRENT_BINARY_DIR}
	# $<INSTALL_INTERFACE:inc>  # <prefix>/inc 
)
target_include_directories(${LIBRARY_NAME_STATIC}  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${INCLUDE_PATH}>
    ${CMAKE_CURRENT_BINARY_DIR}
	# $<INSTALL_INTERFACE:inc>  # <prefix>/inc 
)
target_include_directories(${LIBRARY_NAME_STATIC_PROFILE} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${INCLUDE_PATH}>
    ${CMAKE_CURRENT_BINARY_DIR}
	# $<INSTALL_INTERFACE:inc>  # <prefix>/inc 
)
# Define the install path
message("Installation to path: ${CMAKE_INSTALL_PREFIX}/bin")
message("Installation to path: ${INSTALL_PATH}")


install(TARGETS ${LIBRARY_NAME_SHARED} ${LIBRARY_NAME_STATIC}  ${LIBRARY_NAME_STATIC_PROFILE} easy_profiler DESTINATION "${BINARY_PATH}")
install(TARGETS ${LIBRARY_NAME_SHARED} ${LIBRARY_NAME_STATIC}  ${LIBRARY_NAME_STATIC_PROFILE} DESTINATION "${INSTALL_PATH}")


#DEPLOY_QT(${QT_PATH} "$<TARGET_FILE_DIR:profiler_gui>/$<TARGET_FILE_NAME:profiler_gui>" ${BINARY_PATH})

