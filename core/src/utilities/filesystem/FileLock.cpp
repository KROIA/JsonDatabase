#include "utilities/filesystem/FileLock.h"
#include "utilities/JDUniqueMutexLock.h"
#include "utilities/JDUtilities.h"
#include "utilities/StringUtilities.h"

#ifdef JD_ENABLE_MULTITHREADING
#include <thread>
#endif
//#include <iostream>


namespace JsonDatabase
{
    namespace Internal
    {
        const std::string FileLock::s_lockFileEnding = ".lck";
        std::mutex FileLock::m_mutex;

        FileLock::FileLock(const std::string& filePath, const std::string& fileName)
            : m_directory(Utilities::replaceForwardSlashesWithBackslashes(filePath))
            , m_fileName(fileName)
            , m_locked(false)
            , m_fileHandle(nullptr)
        {
            m_filePath = m_directory + "\\" + m_fileName;
            m_lockFilePathName = m_filePath + s_lockFileEnding;
        }
        FileLock::~FileLock()
        {
            Error err;
            unlock(err);
        }

        const std::string& FileLock::getFilePath() const
        {
            return m_directory;
        }
        const std::string& FileLock::getFileName() const
        {
            return m_fileName;
        }
        bool FileLock::lock(Error& err)
        {
            err = Error::alreadyLocked;
            if (m_locked)
                return true;
            err = lock_internal();
#ifdef JD_DEBUG
            if (err != Error::none)
            {
                JD_CONSOLE_FUNCTION(getErrorStr(err) + "\n");
            }
#endif
            return m_locked;
        }
        bool FileLock::lock(unsigned int timeoutMs, Error& err)
        {
            err = Error::alreadyLocked;
            if (m_locked)
                return true;
            // Get the current time
            auto start = std::chrono::high_resolution_clock::now();

            // Calculate the time point when the desired duration will be reached
            auto end = start + std::chrono::milliseconds(timeoutMs);

            while (std::chrono::high_resolution_clock::now() < end && (err = lock_internal()) != Error::none) {
                JDFILE_FILE_LOCK_PROFILING_BLOCK("FileLock::WaitForFreeLock", JD_COLOR_STAGE_5);
                // Sleep for a short while to avoid busy-waiting
                std::this_thread::yield();
                //std::this_thread::sleep_for(std::chrono::milliseconds(1)); // Adjust as needed
            }
#ifdef JD_DEBUG
            if (err != Error::none)
            {
                JD_CONSOLE_FUNCTION(getErrorStr(err) + "\n");
            }
#endif
            return m_locked;
        }

        bool FileLock::unlock(Error& err)
        {
            err = unlockFile();
            return err == Error::none;
        }

        bool FileLock::isLocked() const
        {
            return m_locked;
        }
        const std::string& FileLock::getErrorStr(Error err)
        {
            switch (err)
            {
            case Error::none: { static const std::string msg = "FileLock::Error::none"; return msg; };
            case Error::unableToCreateOrOpenLockFile: { static const std::string msg = "FileLock::Error::unableToCreateOrOpenLockFile"; return msg; };
            case Error::unableToDeleteLockFile: { static const std::string msg = "FileLock::Error::unableToDeleteLockFile"; return msg; };
            case Error::unableToLock: { static const std::string msg = "FileLock::Error::unableToLock"; return msg; };
            case Error::alreadyLocked: { static const std::string msg = "FileLock::Error::alreadyLocked"; return msg; };
            case Error::alreadyLockedForReading: { static const std::string msg = "FileLock::Error::alreadyLockedForReading"; return msg; };
            case Error::alreadyLockedForWritingByOther: { static const std::string msg = "FileLock::Error::alreadyLockedForWritingByOther"; return msg; };
            case Error::alreadyUnlocked: { static const std::string msg = "FileLock::Error::alreadyUnlocked"; return msg; };

            case Error::lockTimeout: { static const std::string msg = "FileLock::Error::lockTimeout"; return msg; };
            }
            static std::string unknown;
            unknown = "Unknown FileLock Error: " + std::to_string((int)err);
            return unknown;
        }

        bool FileLock::isLockInUse(const std::string& filePath, const std::string& fileName)
        {
			return isFileLocked(filePath + "\\" + fileName + s_lockFileEnding);
        }

        bool FileLock::isFileLocked(const std::string& fullFilePath)
        {
            HANDLE fileHandle = CreateFile(
                fullFilePath.c_str(),
                GENERIC_WRITE,
                0,
                nullptr,
                OPEN_EXISTING,  // Use OPEN_EXISTING instead of CREATE_ALWAYS
                FILE_ATTRIBUTE_NORMAL,
                nullptr
            );

            if (fileHandle == INVALID_HANDLE_VALUE) {
                // Failed to open the file, indicating it might be locked
                return true; // File is possibly locked
            }

            // Try to lock the file
            if (!LockFile(fileHandle, 0, 0, MAXDWORD, MAXDWORD)) {
                // Locking failed, file is already locked
                CloseHandle(fileHandle);
                return true; // File is locked
            }

            // File is not locked, release the lock and close the file handle
            UnlockFile(fileHandle, 0, 0, MAXDWORD, MAXDWORD);
            CloseHandle(fileHandle);

            return false; // File is not locked
        }




        FileLock::Error FileLock::lock_internal()
        {
            //JDFILE_FILE_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_7);
            Error err = lockFile();
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
            return err;
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

        FileLock::Error FileLock::unlockFile()
        {
            JDFILE_FILE_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_8);
            if (!m_locked)
                return Error::alreadyUnlocked;

            Error err = Error::none;



            if (!UnlockFile(m_fileHandle, 0, 0, MAXDWORD, MAXDWORD))
            {
                DWORD error = GetLastError();
                JD_CONSOLE_FUNCTION("Error UnlockFile. GetLastError() =  " << error << " : " << Utilities::getLastErrorString(error) << "\n");
            }

            if (!CloseHandle(m_fileHandle))
            {
                DWORD error = GetLastError();
                JD_CONSOLE_FUNCTION("Error CloseHandle. GetLastError() =  " << error << " : " << Utilities::getLastErrorString(error) << "\n");
            }
            // Now, delete the file
            if (!DeleteFile(m_lockFilePathName.c_str()))
            {
                // Handle error deleting file
                err = Error::unableToDeleteLockFile;
                JD_CONSOLE_FUNCTION(getErrorStr(err) + "\n");
            }


            m_fileHandle = nullptr;
            m_locked = false;
            return err;
        }
    }    
}