## description: Json Database to serialize/deserialize objects
cmake_minimum_required(VERSION 3.1.0)
include(FetchContent)

# Define the git repository and tag to download from
set(LIB_NAME JsonDatabase)
set(GIT_REPO https://github.com/KROIA/JsonDatabase.git)
set(GIT_TAG main)

FetchContent_Declare(
    ${LIB_NAME}
    GIT_REPOSITORY ${GIT_REPO}
    GIT_TAG        ${GIT_TAG}
)

set(JsonDatabase_NO_EXAMPLES True)
set(JsonDatabase_NO_UNITTESTS True)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build JsonDatabase as static library.")
message("Downloading dependency: ${LIB_NAME} from: ${GIT_REPO} tag: ${GIT_TAG}")
FetchContent_MakeAvailable(${LIB_NAME})


# Add this library to the specific profiles of this project
list(APPEND DEPENDENCIES_FOR_SHARED_LIB JsonDatabase_static)
list(APPEND DEPENDENCIES_FOR_STATIC_LIB JsonDatabase_static)
list(APPEND DEPENDENCIES_FOR_STATIC_PROFILE_LIB JsonDatabase_static_profile) # only use for static profiling profile

