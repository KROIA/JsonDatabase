
cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME PROJECT_NAME)   
string(REPLACE " " "_" ProjectId ${PROJECT_NAME})
project(${PROJECT_NAME})


# Macro to search for files with given file ending.
# call:
#   FILE_DIRECTORIES(H_FILES *.h)
#
# all *.h files will be saved in the H_FILES variable
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
    message(STATUS "Using Qt v ${Qt5Widgets_VERSION} in path: ${QT_PATH}")
else()
    message(STATUS "INFO\n\n\tQt5 not found!\n")
endif()


# end QT settings



include_directories(inc)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Get all source files
FILE_DIRECTORIES(H_FILES *.h)
FILE_DIRECTORIES(CPP_FILES *.cpp)

qt5_wrap_cpp(CPP_MOC_FILES ${H_FILES})

set(Files 
    ${CPP_MOC_FILES}
	${H_FILES}
	${CPP_FILES})

add_executable(${PROJECT_NAME} ${Files})


if(JD_PROFILING)
    target_link_libraries(${PROJECT_NAME} JsonDatabase_static_profile)
else()
    target_link_libraries(${PROJECT_NAME} JsonDatabase_static)
endif()
target_compile_definitions(${PROJECT_NAME} PUBLIC BUILD_STATIC)



install(TARGETS ${PROJECT_NAME} DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND "${QT_PATH}/bin/windeployqt.exe"
            --no-compiler-runtime
            --no-translations
            --no-system-d3d-compiler
            --no-opengl-sw
            --no-angle
            --no-webkit2
            --dir "${CMAKE_INSTALL_PREFIX}/bin"
            --pdb
            "$<TARGET_FILE:${PROJECT_NAME}>"
	 	COMMENT "Running windeployqt..." "${QT_PATH}/bin/windeployqt.exe" "${CMAKE_INSTALL_PREFIX}/bin"
    )

    #get_target_property(TARGET_PATH ${PROJECT_NAME} RUNTIME_OUTPUT_DIRECTORY)
    #
    #add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    #    COMMAND "${QT_PATH}/bin/windeployqt.exe"
    #        --no-compiler-runtime
    #        --no-translations
    #        --no-system-d3d-compiler
    #        --no-opengl-sw
    #        --no-angle
    #        --no-webkit2
    #        --dir "${TARGET_PATH}/$<CONFIG>"
    #        --pdb
    #        "$<TARGET_FILE:${PROJECT_NAME}>"
	#	COMMENT "Running windeployqt... ${QT_PATH}/bin/windeployqt.exe ${TARGET_PATH}/$<CONFIG>"
    #)
endif()