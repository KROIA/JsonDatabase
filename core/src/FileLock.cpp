#include "FileLock.h"
#include <thread>
#include <iostream>

namespace JsonDatabase
{
    FileLock::FileLock(const std::string& filePath)
        : m_filePath(filePath) 
        , m_locked(false)
        , m_lastError(Error::none)
#ifdef _WIN32
        , m_fileHandle(nullptr)
#else
        , m_m_fileDescriptor(0);
#endif
    {

    }
    FileLock::~FileLock() 
    {
        unlockFile();
    }

    const std::string& FileLock::getFilePath() const
    {
        return m_filePath;
    }
    bool FileLock::lock()
    {
        m_lastError = Error::none;
        if (!m_locked)
        {
            m_lastError = lockFile();
#ifdef JD_DEBUG
            if (m_lastError != Error::none)
            {
                JD_CONSOLE_FUNCTION(getLastErrorStr() +"\n");
            }
#endif
        }
        JDFILE_FILE_LOCK_PROFILING_VALUE("locked", m_locked);
        if (m_lastError == Error::none)
            return true;
        return false;
    }
    bool FileLock::lock(unsigned int timeoutMs)
    {
        // Get the current time
        auto start = std::chrono::high_resolution_clock::now();

        // Calculate the time point when the desired duration will be reached
        auto end = start + std::chrono::milliseconds(timeoutMs);

        while (std::chrono::high_resolution_clock::now() < end && !lock()) {

            // Sleep for a short while to avoid busy-waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(1)); // Adjust as needed
        }
        return isLocked();
    }
    void FileLock::unlock()
    {
        m_lastError = Error::none;
        unlockFile();
        JDFILE_FILE_LOCK_PROFILING_VALUE("locked", m_locked);
    }

    bool FileLock::isLocked() const
    {
        return m_locked;
    }
    enum FileLock::Error FileLock::getLastError() const
    {
        return m_lastError;
    }
    const std::string& FileLock::getLastErrorStr() const
    {
        switch (m_lastError)
        {
        case Error::none:                               { static const std::string msg = "FileLock::Error::none"; return msg; };
        case Error::unableToCreateOrOpenLockFile:       { static const std::string msg = "FileLock::Error::unableToCreateOrOpenLockFile"; return msg; };
        case Error::unableToDeleteLockFile:             { static const std::string msg = "FileLock::Error::unableToDeleteLockFile"; return msg; };
        case Error::unableToLock:                       { static const std::string msg = "FileLock::Error::unableToLock"; return msg; };
        case Error::alreadyLocked:                      { static const std::string msg = "FileLock::Error::alreadyLocked"; return msg; };
        }
        static std::string unknown;
        unknown = "Unknown FileLock Error: " + std::to_string(m_lastError);
        return unknown;
    }

    FileLock::Error FileLock::lockFile() 
    {
        JDFILE_FILE_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_10);
        if (m_locked)
            return Error::alreadyLocked;
#ifdef _WIN32
        m_fileHandle = CreateFile(
            m_filePath.c_str(),
            GENERIC_WRITE,
            0,
            nullptr,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        );

        if (m_fileHandle == INVALID_HANDLE_VALUE) {
            m_locked = false;
            return Error::unableToCreateOrOpenLockFile;
            //throw std::runtime_error("Unable to create or open file.");
        }

        if (LockFile(m_fileHandle, 0, 0, MAXDWORD, MAXDWORD)) {
            //std::cout << "File locked successfully." << std::endl;
        }
        else {
            m_locked = false;
            return Error::unableToLock;
            //throw std::runtime_error("Unable to lock file.");
        }
#else
        m_fileDescriptor = open(m_filePath.c_str(), O_RDWR | O_CREAT, 0666);

        if (m_fileDescriptor == -1) {
            return Error::unableToCreateOrOpenLockFile;
            //throw std::runtime_error("Unable to create or open file.");
        }

        struct flock fl;
        fl.l_type = F_WRLCK;
        fl.l_whence = SEEK_SET;
        fl.l_start = 0;
        fl.l_len = 0;

        if (fcntl(m_fileDescriptor, F_SETLKW, &fl) == -1) {
            return Error::unableToLock;
            //throw std::runtime_error("Unable to lock file.");
        }

        //std::cout << "File locked successfully." << std::endl;
#endif

        
        m_locked = true;
        return Error::none;
    }

    void FileLock::unlockFile() 
    {
        JDFILE_FILE_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_10);
#ifdef _WIN32
        if (!m_fileHandle) return;

        UnlockFile(m_fileHandle, 0, 0, MAXDWORD, MAXDWORD);
        CloseHandle(m_fileHandle);
        m_fileHandle = nullptr;

        // Now, delete the file
        if (!DeleteFile(m_filePath.c_str())) 
        {
            // Handle error deleting file
            m_lastError = Error::unableToDeleteLockFile;
            JD_CONSOLE_FUNCTION(getLastErrorStr() + "\n");
        }
#else
        if (!m_fileDescriptor) return;
        struct flock fl;
        fl.l_type = F_UNLCK;
        fl.l_whence = SEEK_SET;
        fl.l_start = 0;
        fl.l_len = 0;

        fcntl(m_fileDescriptor, F_SETLKW, &fl);
        close(m_fileDescriptor);
        m_fileDescriptor = 0;

        // Now, delete the file
        if (unlink(m_filePath.c_str()) != 0) {
            // Handle error deleting file
            m_lastError = Error::unableToDeleteLockFile;
            JD_CONSOLE_FUNCTION(getLastErrorStr() + "\n");
        }
#endif
        m_locked = false;

        //std::cout << "File unlocked successfully." << std::endl;
    }
}