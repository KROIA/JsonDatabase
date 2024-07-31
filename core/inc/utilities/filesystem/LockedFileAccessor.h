#pragma once

#include "JsonDatabase_base.h"
#include "JsonDatabase_Declaration.h"
#include "FileReadWriteLock.h"


#include "Json/JsonValue.h"

#include "Logger.h"


namespace JsonDatabase
{
	namespace Internal
	{
		class JSON_DATABASE_EXPORT LockedFileAccessor
		{
		public:
			enum class Error
			{
				none = 0,

				// File locking
				fileLock_unableToCreateOrOpenLockFile = 1,
				fileLock_unableToDeleteLockFile = 2,
				fileLock_unableToLock = 3,
				fileLock_alreadyLocked = 4,
				fileLock_alreadyLockedForReading = 5,
				fileLock_alreadyLockedForWritingByOther = 6,
				fileLock_alreadyUnlocked = 7,
				fileLock_notAquired = 8,

				// Json reading/writing
				json_parseError = 30,
				json_isNotAnObject = 31,
				json_isNotAnArray = 32,

				// File reading/writing
				file_cantOpenFileForRead = 40,
				file_cantOpenFileForWrite = 41,
				file_invalidFileSize = 42,
				file_cantReadFile = 43,
				file_cantWriteFile = 44,
				file_cantVerifyFileContents = 45,


			};

			enum class AccessMode
			{
				read = 1,
				write = 2,
				readWrite = 3
			};

			LockedFileAccessor(const std::string& directory,
							   const std::string& name,
							   const std::string& endig, 
				Log::LogObject *parentLogger);
			~LockedFileAccessor();

			Error lock(AccessMode mode);
			Error lock(AccessMode mode, unsigned int timeoutMillis);
			Error unlock();
			bool isLocked() const;

			void useZipFormat(bool useZipFormat);
			bool useZipFormat() const;

            void setProgress(Internal::WorkProgress* progress);
            Internal::WorkProgress* progress() const;

			std::string getFullFilePath() const;
			std::string getFullFileName() const;


            Error writeJsonFile(const JsonArray& jsons) const;
            Error writeJsonFile(const JsonObject& json) const;


            Error readJsonFile(JsonArray& jsonsOut) const;
            Error readJsonFile(JsonObject& objOut) const;

			Error readFile(QByteArray& fileDataOut) const;
			Error writeFile(const QByteArray& fileData) const;


			static const std::string& getErrorStr(Error err);

		private:
			Error readFile_internal(QByteArray& fileDataOut) const;
			Error writeFile_internal(const QByteArray& fileData) const;

			Log::LogObject* m_logger = nullptr;

			mutable FileReadWriteLock* m_fileLock;
			FileReadWriteLock::Access m_accessMode;

			std::string m_directory;
			std::string m_name;
			std::string m_ending;

			bool m_useZipFormat;

            Internal::WorkProgress* m_progress;
		};
	}
}
