#pragma once

#include "JD_base.h"

#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif


namespace fs = std::filesystem;

namespace JsonDatabase
{
    class JSONDATABASE_EXPORT FileLock {
    public:
        enum Error
        {
            none,
            unableToCreateOrOpenLockFile,
            unableToDeleteLockFile,
            unableToLock,
            alreadyLocked,
        };


        FileLock(const std::string& filePath);

        ~FileLock();

        const std::string& getFilePath() const;

        bool lock();
        bool lock(unsigned int timeoutMs);
        void unlock();

        bool isLocked() const;

        Error getLastError() const;
        const std::string& getLastErrorStr() const;


    private:
        Error lockFile();
        void unlockFile();

        std::string m_filePath;
#ifdef _WIN32
        HANDLE m_fileHandle;
#else
        int m_fileDescriptor;
#endif

        bool m_locked;
        Error m_lastError;
    };
}