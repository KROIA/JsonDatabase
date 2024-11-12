#pragma once

#include "JsonDatabase_base.h"
#include "FileLock.h"

#include <string>
#include <vector>

#include <windows.h>
#include <mutex>

#include "Logger.h"



namespace JsonDatabase
{
	namespace Internal
	{
		class JSON_DATABASE_EXPORT FileReadWriteLock
		{
		public:
			enum Access
			{
				unknown = 0,
				read = 1,
				write = 2,
				readWrite = 3,
			};

			FileReadWriteLock(const std::string& filePath, const std::string& fileName, Log::LogObject *logger);
			~FileReadWriteLock();

			const std::string& getFilePath() const;
			const std::string& getFileName() const;

			bool lock(Access direction, bool& wasLockedByOtherUserOut, Error& err);
			bool lock(Access direction, unsigned int timeoutMs, bool& wasLockedByOtherUserOut, Error& err);
			void unlock(Error& err);

			bool isLocked() const;

			Access getAccessStatus() const;
			Access getAccessStatus(size_t& readerCount) const;
			bool tryDeleteLocks();

			
			static const std::string& accessTypeToString(Access access);
			static Access stringToAccessType(const std::string& accessStr);
			static std::string getRandomString(size_t length);


		private:
			Error lock_internal(Access direction, bool& wasLockedByOtherUserOut);
			Error lockFile(Access direction, bool& wasLockedByOtherUserOut);


			Log::LogObject *m_logger = nullptr;
			//std::string m_filePath;
			std::string m_directory;
			std::string m_fileName;

			std::string m_lockFilePathName;

			bool m_locked;
			Access m_access;

			FileLock* m_lock;

			static const unsigned int s_tryLockTimeoutMs;
		};
	}
}
