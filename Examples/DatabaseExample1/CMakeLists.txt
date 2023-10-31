
cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME PROJECT_NAME)   
string(REPLACE " " "_" ProjectId ${PROJECT_NAME})
project(${PROJECT_NAME})




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


#DEPLOY_QT(${QT_PATH} "$<TARGET_FILE_DIR:${PROJECT_NAME}>/$<TARGET_FILE_NAME:${PROJECT_NAME}>" "$<TARGET_FILE_DIR:${PROJECT_NAME}>")


# set(COMPILER_OUTPUT_PATH "$<TARGET_FILE:${PROJECT_NAME}>/..")



# Copy executable to bin directory
# add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy
#     "$<TARGET_FILE:${PROJECT_NAME}>"
#     "${BINARY_PATH}/${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX}"
# )
# 
# 
# if(WIN32)
#     
#     set(DEPLOY_COMMAND  "${QT_PATH}/bin/windeployqt.exe"
#             --no-compiler-runtime
#             --no-translations
#             --no-system-d3d-compiler
#             --no-opengl-sw
#             --no-angle
#             --no-webkit2
#             --pdb)
# 
#     # Deploy on the bin path
#     add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#         COMMAND ${DEPLOY_COMMAND} 
#             --dir "${BINARY_PATH}"
#             "$<TARGET_FILE:${PROJECT_NAME}>"
# 		COMMENT "Running windeployqt..." "${QT_PATH}/bin/windeployqt.exe" "${BINARY_PATH}"
#     )
# 
#     # deploy on the compiler output path
#     add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#         COMMAND ${DEPLOY_COMMAND}
#             --dir "${COMPILER_OUTPUT_PATH}"
#             "$<TARGET_FILE:${PROJECT_NAME}>"
# 		COMMENT "Running windeployqt..." "${QT_PATH}/bin/windeployqt.exe" "${COMPILER_OUTPUT_PATH}"
#     )
# endif()