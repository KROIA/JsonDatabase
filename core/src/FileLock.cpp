#include "FileLock.h"
#include <thread>
#include <iostream>

#include "JDUniqueMutexLock.h"

namespace JsonDatabase
{
    const std::string FileLock::s_lockFileEnding = ".clk";
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
        return lock_internal();        
    }
    bool FileLock::lock(unsigned int timeoutMs)
    {
        // Get the current time
        auto start = std::chrono::high_resolution_clock::now();

        // Calculate the time point when the desired duration will be reached
        auto end = start + std::chrono::milliseconds(timeoutMs);

        while (std::chrono::high_resolution_clock::now() < end && !lock_internal()) {
            JDFILE_FILE_LOCK_PROFILING_BLOCK("WaitForFreeLock", JD_COLOR_STAGE_5);
            // Sleep for a short while to avoid busy-waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(1)); // Adjust as needed
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
        bool wasLocked = m_locked;
#endif
        m_lastError = Error::none;
        unlockFile();
#ifdef JD_PROFILING
        if (wasLocked)
        {
            JDFILE_FILE_LOCK_PROFILING_END_BLOCK;
        }
        //JDFILE_FILE_LOCK_PROFILING_VALUE("locked", m_locked);
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

        //  JDM_UNIQUE_LOCK;
       


        m_fileHandle = CreateFile(
            m_lockFilePathName.c_str(),
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



        m_locked = true;
        return Error::none;
    }
  /*  FileLock::Error FileLock::lockFile_old(Access direction)
    {
        JDFILE_FILE_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_8);
        if (m_locked)
            return Error::alreadyLocked;

      //  JDM_UNIQUE_LOCK;


        m_access = Access::unknown;

        std::vector<std::string> files = getFileNamesInDirectory(m_directory, s_lockFileEnding);

        size_t readerCount = 0;
        for (const std::string& file : files)
        {
            if (file.find(s_lockFileEnding) == std::string::npos)
                continue;

            // Check the filename to see if it matches the file we want to lock
            size_t pos = file.find_last_of("_");
            if (pos == std::string::npos)
				continue;
            std::string fileName = file.substr(0, pos);

			if (fileName != m_fileName)
				continue;

			// Check the access type
            size_t pos2 = file.find_last_of("-");
            std::string accessType = file.substr(pos + 1, pos2 - pos - 1);
            Access access = stringToAccessType(accessType);
            switch (access)
            {
                case Access::readWrite:
                case Access::write:
                {
                    // Already locked for writing by a other process
                    return Error::alreadyLocked;
                }
                case Access::read:
                {
					++readerCount;
					break;
				}
                case Access::unknown:
                {
                    int a = 0;

                }
            }
        }

        if (direction == Access::write && readerCount != 0)
        {
            // Some are reading, can't write
            return Error::alreadyLocked;
        }


        std::string accessType = accessTypeToString(direction);
        std::string randStr;
        if(direction == Access::read)
			randStr = std::to_string(readerCount)+getRandomString(10);

        std::string lockFileName = m_filePath + "_" + accessType +"-"+ randStr + s_lockFileEnding;

        m_fileHandle = CreateFile(
            lockFileName.c_str(),
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


        
        m_locked = true;
        m_access = direction;
        m_lockFilePathName = lockFileName;
        return Error::none;
    }*/

    void FileLock::unlockFile() 
    {
        JDFILE_FILE_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_8);

        if (!m_fileHandle) return;

        UnlockFile(m_fileHandle, 0, 0, MAXDWORD, MAXDWORD);
        CloseHandle(m_fileHandle);
        m_fileHandle = nullptr;

        // Now, delete the file
        if (!DeleteFile(m_lockFilePathName.c_str()))
        {
            // Handle error deleting file
            m_lastError = Error::unableToDeleteLockFile;
            JD_CONSOLE_FUNCTION(getLastErrorStr() + "\n");
        }


        m_locked = false;

        //std::cout << "File unlocked successfully." << std::endl;
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