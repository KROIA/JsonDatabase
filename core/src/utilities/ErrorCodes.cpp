#include "utilities/ErrorCodes.h"

namespace JsonDatabase
{
	const char* errorToString(Error error)
	{
		switch (error)
		{
			case Error::none:
				return "No error";
			case Error::objIsNullptr:
				return "Object is nullptr";
			case Error::cantOpenRegistryFile:
				return "Can't open registry file";
			case Error::registryFileNotFound:
				return "Registry file not found";
			case Error::corruptRegistryFileData:
				return "Corrupt registry file data";
			case Error::unableToLockObject:
				return "Unable to lock object";
			case Error::unableToUnlockObject:
				return "Unable to unlock object";
			case Error::objectLockedByOther:
				return "Object locked by other session";
			case Error::objectNotLocked:
				return "Object not locked";

			case Error::unableToCreateOrOpenLockFile:
				return "Unable to create or open lock file";
			case Error::unableToDeleteLockFile:
				return "Unable to delete lock file";
			case Error::unableToLockFile:
				return "Unable to lock file";
			case Error::fileAlreadyLocked:
				return "File already locked";
			case Error::fileAlreadyLockedForReading:
				return "File already locked for reading";
			case Error::fileAlreadyLockedForWritingByOther:
				return "File already locked for writing by other session";
			case Error::fileAlreadyUnlocked:
				return "File already unlocked";
			case Error::fileLockTimeout:
				return "File lock timeout";
			case Error::fileNotLocked:
				return "File not locked";

			case Error::cantOpenFileForRead:
				return "Can't open file for read";
			case Error::cantOpenFileForWrite:
				return "Can't open file for write";
			case Error::invalidFileSize:
				return "Invalid file size";
			case Error::cantReadFile:
				return "Can't read file";
			case Error::cantWriteFile:
				return "Can't write file";
			case Error::cantVerifyFileContents:
				return "Can't verify file contents";


			case Error::__programmingError:
				return "Programming error";
		}
		return "Unknown error";
	}
}