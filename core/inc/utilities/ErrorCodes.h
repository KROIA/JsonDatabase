#pragma once

#include "JsonDatabase_base.h"

namespace JsonDatabase
{
	enum Error
	{
		none,
		objIsNullptr,
		cantOpenRegistryFile,
		registryFileNotFound,
		corruptRegistryFileData,
		unableToLockObject,
		unableToUnlockObject,
		objectLockedByOther,
		objectNotLocked,

		unableToCreateOrOpenLockFile,
		unableToDeleteLockFile,
		unableToLockFile,
		fileAlreadyLocked,
		fileAlreadyLockedForReading,
		fileAlreadyLockedForWritingByOther,
		fileAlreadyUnlocked,
		fileLockTimeout,
		fileNotLocked,

		cantOpenFileForRead,
		cantOpenFileForWrite,
		invalidFileSize,
		cantReadFile,
		cantWriteFile,
		cantVerifyFileContents,

		__programmingError,
		__count
	};

	extern const char* errorToString(Error err);
}