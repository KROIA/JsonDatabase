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

        FileLock::FileLock(const std::string& filePath, const std::string& fileName, Log::LogObject *logger)
            : m_logger(logger)
            , m_directory(Utilities::replaceForwardSlashesWithBackslashes(filePath))
            , m_fileName(fileName)
            , m_locked(false)
            , m_fileHandle(nullptr)
        {
            //m_filePath = m_directory + "\\" + m_fileName;
            m_lockFilePathName = getFullFilePath(m_directory, m_fileName);
            //m_lockFilePathName = m_filePath + s_lockFileEnding;
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
        bool FileLock::tryGetLock(Error& err)
        {
            HANDLE fileHandle = CreateFile(
#ifdef UNICODE
                Utilities::strToWstr(m_lockFilePathName).c_str(),
#else
                m_lockFilePathName.c_str(),
#endif
                GENERIC_WRITE,
                0,
                nullptr,
                OPEN_EXISTING,  // Use OPEN_EXISTING instead of CREATE_ALWAYS
                FILE_ATTRIBUTE_NORMAL,
                nullptr
            );

            if (fileHandle == INVALID_HANDLE_VALUE) {
                // Failed to open the file, indicating it might be locked
                m_locked = false;
                err = Error::fileAlreadyLocked;
                return false; // File is possibly locked
            }

            if (!LockFile(fileHandle, 0, 0, MAXDWORD, MAXDWORD))
            {
                m_locked = false;
                CloseHandle(fileHandle);
               
                err = Error::unableToLockFile;
            }
            m_fileHandle = fileHandle;
            m_locked = true;
            err = Error::none;
            return true;            
        }
        bool FileLock::lock(Error& err)
        {
            err = Error::fileAlreadyLocked;
            if (m_locked)
                return true;
            err = lock_internal();
#ifdef JD_DEBUG
            if (err != Error::none)
            {
                if(m_logger)m_logger->logError(errorToString(err));
            }
#endif
            return m_locked;
        }
        bool FileLock::lock(unsigned int timeoutMs, Error& err)
        {
            err = Error::fileAlreadyLocked;
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
                if(m_logger)m_logger->logError(errorToString(err));
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

        bool FileLock::isLockInUse(const std::string& filePath, const std::string& fileName)
        {
            if (!fileExists(filePath, fileName))
                return false; // File doesn't exist, so it can't be locked
			return isFileLocked(getFullFilePath(filePath, fileName));
        }

        bool FileLock::isFileLocked(const std::string& fullFilePath)
        {
            
            HANDLE fileHandle = CreateFile(
#ifdef UNICODE
                Utilities::strToWstr(fullFilePath).c_str(),
#else
                fullFilePath.c_str(),
#endif
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
        bool FileLock::deleteFile(const std::string& fullFilePath)
        {
#ifdef UNICODE
            return DeleteFile(Utilities::strToWstr(fullFilePath).c_str());
#else
            return DeleteFile(fullFilePath.c_str());
#endif
        }

        bool FileLock::fileExists(const std::string& filePath, const std::string& fileName)
        {
            return fileExists(getFullFilePath(filePath, fileName));
        }
        bool FileLock::fileExists(const std::string& fullFilePath)
        {
#ifdef UNICODE
            DWORD fileAttributes = GetFileAttributes(Utilities::strToWstr(fullFilePath).c_str());
#else
            DWORD fileAttributes = GetFileAttributes(fullFilePath.c_str());
#endif            
            if (fileAttributes == INVALID_FILE_ATTRIBUTES) {
                if (GetLastError() == ERROR_FILE_NOT_FOUND || GetLastError() == ERROR_PATH_NOT_FOUND) {
                    return false; // File doesn't exist
                }
            }
            return true; // File exists
        }

        bool FileLock::deleteFile(const std::string& filePath, const std::string& fileName)
        {
            return deleteFile(getFullFilePath(filePath, fileName));
        }
        std::vector<std::string> FileLock::getLockFileNamesInDirectory(const std::string& directory)
        {
			return getFileNamesInDirectory(directory, s_lockFileEnding);
        }



        std::vector<std::string> FileLock::getFileNamesInDirectory(const std::string& directory)
        {
            std::vector<std::string> fileNames;

            WIN32_FIND_DATA findFileData;
            
#ifdef UNICODE
            std::wstring wDirectory = Utilities::strToWstr(directory);
            wDirectory += L"\\*";
            HANDLE hFind = FindFirstFile(wDirectory.c_str(), &findFileData);
#else
            HANDLE hFind = FindFirstFile((directory+"\\*").c_str(), &findFileData);
#endif 
            

            if (hFind == INVALID_HANDLE_VALUE)
            {
                return fileNames;
            }

            do
            {
                if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    // This is a directory, skip it
                    continue;
                }
#ifdef UNICODE
                fileNames.push_back(Utilities::wstrToStr(findFileData.cFileName));
#else
                fileNames.push_back(findFileData.cFileName);
#endif 
                
            } while (FindNextFile(hFind, &findFileData) != 0);

            FindClose(hFind);

            return fileNames;
        }
        std::vector<std::string> FileLock::getFileNamesInDirectory(const std::string& directory, const std::string& fileEndig)
        {
            std::vector<std::string> fileNames;

            WIN32_FIND_DATA findFileData;
#ifdef UNICODE
            HANDLE hFind = FindFirstFile(Utilities::strToWstr(directory + "\\*" + fileEndig).c_str(), &findFileData);
#else
            HANDLE hFind = FindFirstFile((directory + "\\*" + fileEndig).c_str(), &findFileData);
#endif 
            

            if (hFind == INVALID_HANDLE_VALUE)
            {
                return fileNames;
            }

            do
            {
                if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    // This is a directory, skip it
                    continue;
                }
#ifdef UNICODE
                std::string fileFullName = Utilities::wstrToStr(findFileData.cFileName);
#else
                std::string fileFullName = findFileData.cFileName;
#endif 
                
                std::string fileName = fileFullName.substr(0, fileFullName.size() - fileEndig.size());
                fileNames.push_back(fileName);
            } while (FindNextFile(hFind, &findFileData) != 0);

            FindClose(hFind);

            return fileNames;
        }
        std::string FileLock::getFullFilePath(const std::string& filePath, const std::string& fileName)
        {
            return filePath + "\\" + fileName + s_lockFileEnding;
        }



        Error FileLock::lock_internal()
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
        Error FileLock::lockFile()
        {
            JDFILE_FILE_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_8);
            if (m_locked)
                return Error::fileAlreadyLocked;

            m_fileHandle = CreateFile(
#ifdef UNICODE
                Utilities::strToWstr(m_lockFilePathName).c_str(),
#else
                m_lockFilePathName.c_str(),
#endif 
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
                return Error::unableToLockFile;
            }
            m_locked = true;
            return Error::none;
        }

        Error FileLock::unlockFile()
        {
            JDFILE_FILE_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_8);
            if (!m_locked)
                return Error::fileAlreadyUnlocked;

            Error err = Error::none;

            DWORD error1 = 0;
            DWORD error2 = 0;

            if (!UnlockFile(m_fileHandle, 0, 0, MAXDWORD, MAXDWORD))
            {
                error1 = GetLastError();
                if(m_logger)m_logger->logError("UnlockFile. GetLastError() =  " + std::to_string(error1) + " : " + Utilities::getLastErrorString(error1));
            }

            if (!CloseHandle(m_fileHandle))
            {
                error2 = GetLastError();
                if(m_logger)m_logger->logError("CloseHandle. GetLastError() =  " + std::to_string(error2) + " : " + Utilities::getLastErrorString(error2));
            }


            // Now, delete the file
            if(!deleteFile(m_lockFilePathName.c_str()))
           // if (!DeleteFile(m_lockFilePathName.c_str()))
            {
                DWORD error3 = GetLastError();
                if (error3 != 32 && // File already used by another process (OK since after freeing the lock, a other process could have locked it)
                    error3 != 2)    // File not found (OK since the file could have been deleted by another process)
                {
                    std::string errorStr = Utilities::getLastErrorString(error3);
                    if (!isLockInUse(m_directory, m_fileName) && fileExists(m_lockFilePathName))
                    {
                        // Handle error deleting file
                        err = Error::unableToDeleteLockFile;
                        if (m_logger)m_logger->logError("Deleting lock file: " + m_lockFilePathName + " " + errorToString(err));
                    }
                }
            }


            m_fileHandle = nullptr;
            m_locked = false;
            return err;
        }
    }    
}