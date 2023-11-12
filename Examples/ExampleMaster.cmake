cmake_minimum_required(VERSION 3.20.0)

# This functions creates a default example project using the given library
# Function name: exampleMaster
# Params: PARENT_LIBRARY_NAME   Name of the library. Example: JsonDatabase
#         PROFILING_NAME        Name of the macro that defines that the profiler is enabled. Example: JD_PROFILING
#         QT_ENABLE             Enables QT modules. ON if QT is used, otherwise OFF
#         QT_DEPLOY             Deploys the compiled file to the compile output path and the install path. ON / OFF
#         QT_MODULES            Defines which QT modules are required. 
#                                     set(QT_MODULES
#                                         Core
#                                         # Widgets
#                                         # Gui
#                                         # Network
#                                         # Add any other required modules here
#                                     )
#         INSTALL_BIN_PATH      Specifies the install path. Example: "${CMAKE_INSTALL_PREFIX}/bin"
function(exampleMaster 
			PARENT_LIBRARY_NAME 
			PROFILING_NAME 
			QT_ENABLE 
			QT_DEPLOY 
			QT_MODULES
            INSTALL_BIN_PATH)

			
set(PARENT_LIBRARY_STATIC ${PARENT_LIBRARY_NAME}_static)
set(PARENT_LIBRARY_STATIC_PROFILE ${PARENT_LIBRARY_STATIC}_profile)

# Set the project name to the same as the folder name this file is contained in
cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME PROJECT_NAME)   
string(REPLACE " " "_" ProjectId ${PROJECT_NAME})

# Change the project name if it gets compiled with profiler enabled
if(${PROFILING_NAME})
    set(PROJECT_NAME ${PROJECT_NAME}_profile)
    message("Profiling is enabled for ${PROJECT_NAME}")
else()
    message("Profiling is disabled for ${PROJECT_NAME}")
endif()
project(${PROJECT_NAME})



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

include_directories(inc)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Get all source files
FILE_DIRECTORIES(H_FILES *.h)
FILE_DIRECTORIES(CPP_FILES *.cpp)


set(SOURCES 
	${H_FILES}
	${CPP_FILES})

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

add_executable(${PROJECT_NAME} ${SOURCES})


if(${PROFILING_NAME})
    target_link_libraries(${PROJECT_NAME} ${PARENT_LIBRARY_STATIC_PROFILE} ${QT_LIBS})
    target_compile_definitions(${PROJECT_NAME}   PRIVATE  ${PROFILING_NAME})
else()
    target_link_libraries(${PROJECT_NAME} ${PARENT_LIBRARY_STATIC} ${QT_LIBS})
endif()
target_compile_definitions(${PROJECT_NAME} PUBLIC BUILD_STATIC)

install(TARGETS ${PROJECT_NAME} DESTINATION "${INSTALL_BIN_PATH}")

if(QT_ENABLE AND QT_DEPLOY)
   DEPLOY_QT(${QT_PATH} ${PROJECT_NAME} "$<TARGET_FILE_DIR:${PROJECT_NAME}>/$<TARGET_FILE_NAME:${PROJECT_NAME}>" "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
   DEPLOY_QT(${QT_PATH} ${PROJECT_NAME} "$<TARGET_FILE_DIR:${PROJECT_NAME}>/$<TARGET_FILE_NAME:${PROJECT_NAME}>" "${INSTALL_BIN_PATH}")
   
  #set(outputPath "${INSTALL_BIN_PATH}")
  #set(targetExePath "$<TARGET_FILE_DIR:${PROJECT_NAME}>/$<TARGET_FILE_NAME:${PROJECT_NAME}>")
  ##get_target_property(targetExePath TARGET ${PROJECT_NAME} PROPERTY LOCATION)
  #
  #message("EXE path = ${targetExePath}")
  #set(DEPLOY_COMMAND  "${QT_PATH}/bin/windeployqt.exe"
  #             --no-compiler-runtime
  #             --no-translations
  #             --no-system-d3d-compiler
  #             --no-opengl-sw
  #             --no-angle
  #             --no-webkit2
  #             --pdb)
  #
  #
  #string(RANDOM LENGTH 20 UNIQUE_TARGET_NAME)
  #message("DummyTargetName: ${UNIQUE_TARGET_NAME}")
  #
  #add_custom_target(${UNIQUE_TARGET_NAME} ALL
  #    DEPENDS "${PROJECT_NAME}"
  #)
  # # Deploy easy_profiler gui to bin folder
  #  add_custom_command(TARGET ${UNIQUE_TARGET_NAME}
  #      COMMAND ${DEPLOY_COMMAND} 
  #          --dir "${outputPath}"
  #         "${targetExePath}"
 #    COMMENT "Running windeployqt..." "${QT_PATH}/bin/windeployqt.exe" "${outputPath}"
  #     DEPENDS UNIQUE_TARGET_NAME
  #  )
endif()

endfunction()