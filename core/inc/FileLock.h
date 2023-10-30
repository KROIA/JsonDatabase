#pragma once

#include "JD_global.h"

#include <fstream>
#include <mutex>
#include <chrono>
#include <thread>

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
            unableToLock,
            alreadyLocked
        };


        FileLock(const std::string& filePath);

        ~FileLock();

        bool lock();
        void unlock();

        bool isLocked() const;
        enum Error getLastError() const;

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