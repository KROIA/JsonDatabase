#pragma once

#include "JsonDatabase_base.h"

#include <string>
#include <vector>

#include <windows.h>
#include <mutex>



namespace fs = std::filesystem;

namespace JsonDatabase
{
    namespace Internal
    {
        class JSON_DATABASE_EXPORT_EXPORT FileLock {
        public:
            enum Error
            {
                none = 0,

                unableToCreateOrOpenLockFile = 1,
                unableToDeleteLockFile = 2,
                unableToLock = 3,
                alreadyLocked = 4,
                alreadyLockedForReading = 5,
                alreadyLockedForWritingByOther = 6,
                alreadyUnlocked = 7,

                lockTimeout = 20,
            };
            FileLock(const std::string& filePath, const std::string& fileName);
            ~FileLock();

            const std::string& getFilePath() const;
            const std::string& getFileName() const;

            bool tryGetLock(Error& err);
            bool lock(Error& err);
            bool lock(unsigned int timeoutMs, Error& err);
            bool unlock(Error& err);

            bool isLocked() const;


            static const std::string& getErrorStr(Error err);
            static bool isLockInUse(const std::string& filePath, const std::string& fileName);
            
            static bool fileExists(const std::string& filePath, const std::string& fileName);
            static bool deleteFile(const std::string& filePath, const std::string& fileName);
            static std::vector<std::string> getLockFileNamesInDirectory(const std::string& directory);
            static std::string getFullFilePath(const std::string& filePath, const std::string& fileName);

            static std::vector<std::string> getFileNamesInDirectory(const std::string& directory);
            static std::vector<std::string> getFileNamesInDirectory(const std::string& directory, const std::string& fileEndig);

            static const std::string s_lockFileEnding;
        private:
            static bool isFileLocked(const std::string& fullFilePath);
            static bool deleteFile(const std::string& fullFilePath);
            Error lock_internal();
            Error lockFile();
            Error unlockFile();




            //std::string m_filePath;
            std::string m_directory;
            std::string m_fileName;

            std::string m_lockFilePathName;

            HANDLE m_fileHandle;



            bool m_locked;


            static std::mutex m_mutex;
        };
    }
}