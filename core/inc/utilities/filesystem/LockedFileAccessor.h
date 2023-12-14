#pragma once

#include "JD_base.h"
#include "JDDeclaration.h"
#include "FileReadWriteLock.h"

#if JD_ACTIVE_JSON == JD_JSON_QT
#include <QJsonObject>
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
#include "Json/JsonValue.h"
#endif

namespace JsonDatabase
{
	namespace Internal
	{
		class JSONDATABASE_EXPORT LockedFileAccessor
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
							   const std::string& endig);
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


#if JD_ACTIVE_JSON == JD_JSON_QT
            Error writeJsonFile(const std::vector<QJsonObject>& jsons) const;
            Error writeJsonFile(const QJsonObject& json) const;


            Error readJsonFile(std::vector<QJsonObject>& jsonsOut) const;
            Error readJsonFile(QJsonObject& objOut) const;

#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
            Error writeJsonFile(const JsonArray& jsons) const;
            Error writeJsonFile(const JsonObject& json) const;


            Error readJsonFile(JsonArray& jsonsOut) const;
            Error readJsonFile(JsonObject& objOut) const;
#endif
			Error readFile(QByteArray& fileDataOut) const;
			Error writeFile(const QByteArray& fileData) const;


			static const std::string& getErrorStr(Error err);

		private:
			Error readFile_internal(QByteArray& fileDataOut) const;
			Error writeFile_internal(const QByteArray& fileData) const;

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