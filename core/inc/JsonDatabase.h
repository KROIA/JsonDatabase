// @file JsonDatabase.h
// @brief Main public header for the library.
//
// Include this single header to access the entire public API.
// Add your own public headers inside USER_SECTION 2 so that
// consumers only need `#include "JsonDatabase.h"`.
#pragma once

/// USER_SECTION_START 1

/// USER_SECTION_END

#include "JsonDatabase_info.h"

/// USER_SECTION_START 2
#include "utilities/JDSerializable.h"
#include "object/JDObjectInterface.h"
#include "manager/JDManager.h"


#include "utilities/filesystem/FileChangeWatcher.h"

#include "Json/JsonDeserializer.h"
#include "Json/JsonSerializer.h"
#include "Json/JsonValue.h"
/// USER_SECTION_END