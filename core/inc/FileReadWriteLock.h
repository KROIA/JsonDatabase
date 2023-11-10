#pragma once

#include "JD_base.h"
#include "FileLock.h"

#include <string>
#include <vector>

#include <windows.h>
#include <mutex>



namespace JsonDatabase
{
	class JSONDATABASE_EXPORT FileReadWriteLock 
	{
	public:
		enum Access
		{
			read,
			write,
			readWrite,
			unknown
		};

		FileReadWriteLock(const std::string& filePath, const std::string& fileName);
		~FileReadWriteLock();

		const std::string& getFilePath() const;
		const std::string& getFileName() const;

		bool lock(Access direction);
		bool lock(Access direction, unsigned int timeoutMs);
		void unlock();

		bool isLocked() const;

		Access getAccessStatus() const;
		Access getAccessStatus(size_t &readerCount) const;

		FileLock::Error getLastError() const;
		const std::string& getLastErrorStr() const;

		static std::vector<std::string> getFileNamesInDirectory(const std::string& directory);
		static std::vector<std::string> getFileNamesInDirectory(const std::string& directory, const std::string& fileEndig);
		static const std::string& accessTypeToString(Access access);
		static Access stringToAccessType(const std::string& accessStr);
		static std::string getRandomString(size_t length);


	private:
		bool lock_internal(Access direction);
		FileLock::Error lockFile(Access direction);

		//std::string m_filePath;
		std::string m_directory;
		std::string m_fileName;

		std::string m_lockFilePathName;


		FileLock::Error m_lastError;
		bool m_locked;
		Access m_access;

		FileLock *m_lock;

		static const unsigned int s_tryLockTimeoutMs;
	};
}