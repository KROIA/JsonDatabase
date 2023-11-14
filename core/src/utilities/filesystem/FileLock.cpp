#include "utilities/filesystem/FileLock.h"
#include "utilities/JDUniqueMutexLock.h"

#include <thread>
#include <iostream>


namespace JsonDatabase
{
    const std::string FileLock::s_lockFileEnding = ".lck";
    std::mutex FileLock::m_mutex;

    FileLock::FileLock(const std::string& filePath, const std::string &fileName)
        : m_directory(replaceForwardSlashesWithBackslashes(filePath))
        , m_fileName(fileName)
        , m_locked(false)
        , m_lastError(Error::none)
        , m_fileHandle(nullptr)
    {
        m_filePath = m_directory + "\\" + m_fileName;
        m_lockFilePathName = m_filePath + s_lockFileEnding;
    }
    FileLock::~FileLock() 
    {
        unlock();
    }

    const std::string& FileLock::getFilePath() const
    {
        return m_directory;
    }
    const std::string& FileLock::getFileName() const
    {
        return m_fileName;
    }
    bool FileLock::lock()
    {
        if (m_locked)
            return true;
        return lock_internal();        
    }
    bool FileLock::lock(unsigned int timeoutMs)
    {
        if (m_locked)
            return true;
        // Get the current time
        auto start = std::chrono::high_resolution_clock::now();

        // Calculate the time point when the desired duration will be reached
        auto end = start + std::chrono::milliseconds(timeoutMs);

        while (std::chrono::high_resolution_clock::now() < end && !lock_internal()) {
            JDFILE_FILE_LOCK_PROFILING_BLOCK("FileLock::WaitForFreeLock", JD_COLOR_STAGE_5);
            // Sleep for a short while to avoid busy-waiting
            std::this_thread::yield();
            //std::this_thread::sleep_for(std::chrono::milliseconds(1)); // Adjust as needed
        }
        return m_locked;
    }
    bool FileLock::lock_internal()
    {
        //JDFILE_FILE_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_7);
        m_lastError = Error::none;
        if (!m_locked)
        {
            m_lastError = lockFile();
#ifdef JD_DEBUG
            if (m_lastError != Error::none)
            {
                JD_CONSOLE_FUNCTION(getLastErrorStr() + "\n");
            }
#endif
        }
#ifdef JD_PROFILING
        if (m_locked)
        {
           // JDFILE_FILE_LOCK_PROFILING_NONSCOPED_BLOCK("file locked", JD_COLOR_STAGE_10);
            JDFILE_FILE_LOCK_PROFILING_TEXT("locked", "true");
        }
        else
        {
            JDFILE_FILE_LOCK_PROFILING_TEXT("locked", "false");
        }
#endif
        if (m_lastError == Error::none)
        {
            return true;
        }
        return false;
    }
    void FileLock::unlock()
    {
#ifdef JD_PROFILING
        //JDFILE_FILE_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_7);
        //bool wasLocked = m_locked;
#endif
        m_lastError = Error::none;
        unlockFile();
#ifdef JD_PROFILING
        /*if (wasLocked)
        {
            JDFILE_FILE_LOCK_PROFILING_END_BLOCK;
        }*/
#endif
        
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
        case Error::alreadyLockedForReading:            { static const std::string msg = "FileLock::Error::alreadyLockedForReading"; return msg; };
        case Error::alreadyLockedForWriting:            { static const std::string msg = "FileLock::Error::alreadyLockedForWriting"; return msg; };
        }
        static std::string unknown;
        unknown = "Unknown FileLock Error: " + std::to_string(m_lastError);
        return unknown;
    }

    FileLock::Error FileLock::lockFile()
    {
        JDFILE_FILE_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_8);
        if (m_locked)
            return Error::alreadyLocked;

        //  JDM_UNIQUE_LOCK_P;
       


        m_fileHandle = CreateFile(
            m_lockFilePathName.c_str(),
            GENERIC_WRITE,
            0,
            nullptr,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        );

        if (m_fileHandle == INVALID_HANDLE_VALUE) 
        {
            m_locked = false;
            m_fileHandle = nullptr;
            return Error::unableToCreateOrOpenLockFile;
        }

        if (!LockFile(m_fileHandle, 0, 0, MAXDWORD, MAXDWORD)) 
        {
            m_locked = false;
            CloseHandle(m_fileHandle);
            m_fileHandle = nullptr;
            return Error::unableToLock;
        }
        m_locked = true;
        return Error::none;
    }

    void FileLock::unlockFile() 
    {
        JDFILE_FILE_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_8);

        if (!m_locked) return;

        UnlockFile(m_fileHandle, 0, 0, MAXDWORD, MAXDWORD);
        CloseHandle(m_fileHandle);
        

        // Now, delete the file
        if (!DeleteFile(m_lockFilePathName.c_str()))
        {
            // Handle error deleting file
            m_lastError = Error::unableToDeleteLockFile;
            JD_CONSOLE_FUNCTION(getLastErrorStr() + "\n");
        }

        m_fileHandle = nullptr;
        m_locked = false;
    }


    

    std::string FileLock::replaceForwardSlashesWithBackslashes(const std::string& input)
    {
        std::string result = input;
        size_t pos = 0;

        while ((pos = result.find('/', pos)) != std::string::npos)
        {
            result.replace(pos, 1, "\\");
            pos += 2; // Move past the double backslashes
        }

        return result;
    }
    

    
}