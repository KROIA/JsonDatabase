#pragma once

#include "JD_base.h"

#include <string>
#include <vector>

#include <windows.h>
#include <mutex>



namespace fs = std::filesystem;

namespace JsonDatabase
{
    namespace Internal
    {
        class JSONDATABASE_EXPORT FileLock {
        public:
            enum Error
            {
                none = 0,

                unableToCreateOrOpenLockFile = 1,
                unableToDeleteLockFile = 2,
                unableToLock = 3,
                alreadyLocked = 4,
                alreadyLockedForReading = 5,
                alreadyLockedForWriting = 6,
                alreadyUnlocked = 7,

                lockTimeout = 20,
            };



            FileLock(const std::string& filePath, const std::string& fileName);

            ~FileLock();

            const std::string& getFilePath() const;
            const std::string& getFileName() const;

            bool lock(Error& err);
            bool lock(unsigned int timeoutMs, Error& err);
            bool unlock(Error& err);

            bool isLocked() const;


            static const std::string& getErrorStr(Error err);


            static std::string replaceForwardSlashesWithBackslashes(const std::string& input);



            static const std::string s_lockFileEnding;
        private:
            Error lock_internal();
            Error lockFile();
            Error unlockFile();




            std::string m_filePath;
            std::string m_directory;
            std::string m_fileName;

            std::string m_lockFilePathName;

            HANDLE m_fileHandle;



            bool m_locked;


            static std::mutex m_mutex;
        };
    }
}