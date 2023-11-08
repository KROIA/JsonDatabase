#include "FileReadWriteLock.h"
#include <random>

namespace JsonDatabase
{
    const unsigned int FileReadWriteLock::s_tryLockTimeoutMs = 1000;


    FileReadWriteLock::FileReadWriteLock(const std::string& filePath, const std::string& fileName)
        : m_directory(FileLock::replaceForwardSlashesWithBackslashes(filePath))
        , m_fileName(fileName)
        , m_locked(false)
        , m_access(Access::unknown)
        , m_lock(nullptr)
    {
       // m_filePath = m_directory + "\\" + m_fileName;
    }
    FileReadWriteLock::~FileReadWriteLock()
    {
        unlock();
    }

    const std::string& FileReadWriteLock::getFilePath() const
    {
        return m_directory;
    }
    const std::string& FileReadWriteLock::getFileName() const
    {
        return m_fileName;
    }
    bool FileReadWriteLock::lock(Access direction)
    {
        return lock_internal(direction);
    }
    bool FileReadWriteLock::lock(Access direction, unsigned int timeoutMs)
    {
        m_lastError = FileLock::Error::none;
        if (m_locked)
        {
            m_lastError = FileLock::Error::alreadyLocked;
            return true;
        }

        // Get the current time
        auto start = std::chrono::high_resolution_clock::now();

        // Calculate the time point when the desired duration will be reached
        auto end = start + std::chrono::milliseconds(timeoutMs);


        while (std::chrono::high_resolution_clock::now() < end && !lock_internal(direction)) 
        {
            JDFILE_FILE_LOCK_PROFILING_BLOCK("FileReadWriteLock::WaitForFreeLock", JD_COLOR_STAGE_5);
            // Sleep for a short while to avoid busy-waiting
            std::this_thread::yield();
            //std::this_thread::sleep_for(std::chrono::microseconds(1)); // Adjust as needed
        }
        return m_locked;
    }
    bool FileReadWriteLock::lock_internal(Access direction)
    {
        //JDFILE_FILE_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_7);
        m_lastError = lockFile(direction);
#ifdef JD_DEBUG
        if (m_lastError != FileLock::Error::none)
        {
            JD_CONSOLE_FUNCTION(getLastErrorStr() + "\n");
        }
#endif
        
#ifdef JD_PROFILING
        if (m_locked)
        {
            switch (m_access)
            {
            case Access::read:
            {
                //JDFILE_FILE_LOCK_PROFILING_TEXT("locked", "true reading");
                JDFILE_FILE_LOCK_PROFILING_NONSCOPED_BLOCK("file locked for reading", JD_COLOR_STAGE_10);
                break;
            }
            case Access::write:
            {
                //JDFILE_FILE_LOCK_PROFILING_TEXT("locked", "true writing");
                JDFILE_FILE_LOCK_PROFILING_NONSCOPED_BLOCK("file locked for writing", JD_COLOR_STAGE_10);
                break;
            }
            case Access::readWrite:
            {
                //JDFILE_FILE_LOCK_PROFILING_TEXT("locked", "true read & write");
                JDFILE_FILE_LOCK_PROFILING_NONSCOPED_BLOCK("file locked for read & write", JD_COLOR_STAGE_10);
                break;
            }
            default:
            {
                //JDFILE_FILE_LOCK_PROFILING_TEXT("locked", "true unknown direction");
                JDFILE_FILE_LOCK_PROFILING_NONSCOPED_BLOCK("file locked for unknown operations", JD_COLOR_STAGE_10);
            }
            }
        }
        else
        {
            JDFILE_FILE_LOCK_PROFILING_TEXT("locked", "false");
        }
#endif
        if (m_lastError == FileLock::Error::none)
        {
            return true;
        }
        return false;
    }
    FileLock::Error FileReadWriteLock::lockFile(Access direction)
    {
        if (m_lock)
            return FileLock::Error::alreadyLocked;

        FileLock tmpLock(m_directory, m_fileName);
        if (!tmpLock.lock(s_tryLockTimeoutMs))
            return FileLock::Error::unableToLock;
        
        m_access = Access::unknown;

        std::vector<std::string> files = getFileNamesInDirectory(m_directory, FileLock::s_lockFileEnding);

        size_t readerCount = 0;
        for (const std::string& file : files)
        {
            if (file.find(FileLock::s_lockFileEnding) == std::string::npos)
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
                    return FileLock::Error::alreadyLocked;
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
            return FileLock::Error::alreadyLocked;
        }


        std::string accessType = accessTypeToString(direction);
        std::string randStr;
        if (direction == Access::read)
            randStr = std::to_string(readerCount) + getRandomString(10);

        std::string lockFileName = m_fileName + "_" + accessType + "-" + randStr + FileLock::s_lockFileEnding;

        m_lock = new FileLock(m_directory, lockFileName);

        if (!m_lock->lock())
        {
            FileLock::Error err = m_lock->getLastError();
            delete m_lock;
            m_lock = nullptr;
            return err;
        }

        m_locked = true;
        m_access = direction;
        m_lockFilePathName = lockFileName;
        return FileLock::Error::none;
    }
    void FileReadWriteLock::unlock()
    {
#ifdef JD_PROFILING
        //JDFILE_FILE_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_7);
        bool wasLocked = m_locked;
#endif
        m_lastError = FileLock::Error::none;
        if (m_lock)
        {
            m_locked = false;
            m_lock->unlock();
            delete m_lock;
            m_lock = nullptr;
        }
#ifdef JD_PROFILING
        if (wasLocked)
        {
            JDFILE_FILE_LOCK_PROFILING_END_BLOCK;
        }
        //JDFILE_FILE_LOCK_PROFILING_VALUE("locked", m_locked);
#endif

    }

    bool FileReadWriteLock::isLocked() const
    {
        return m_locked;
    }
    enum FileLock::Error FileReadWriteLock::getLastError() const
    {
        return m_lastError;
    }
    const std::string& FileReadWriteLock::getLastErrorStr() const
    {
        switch (m_lastError)
        {
        case FileLock::Error::none: { static const std::string msg = "FileReadWriteLock::Error::none"; return msg; };
        case FileLock::Error::unableToCreateOrOpenLockFile: { static const std::string msg = "FileReadWriteLock::Error::unableToCreateOrOpenLockFile"; return msg; };
        case FileLock::Error::unableToDeleteLockFile: { static const std::string msg = "FileReadWriteLock::Error::unableToDeleteLockFile"; return msg; };
        case FileLock::Error::unableToLock: { static const std::string msg = "FileReadWriteLock::Error::unableToLock"; return msg; };
        case FileLock::Error::alreadyLocked: { static const std::string msg = "FileReadWriteLock::Error::alreadyLocked"; return msg; };
        }
        static std::string unknown;
        unknown = "Unknown FileReadWriteLock Error: " + std::to_string(m_lastError);
        return unknown;
    }


    std::vector<std::string> FileReadWriteLock::getFileNamesInDirectory(const std::string& directory)
    {
        std::vector<std::string> fileNames;

        WIN32_FIND_DATA findFileData;
        HANDLE hFind = FindFirstFile((directory + "\\*").c_str(), &findFileData);

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
            fileNames.push_back(findFileData.cFileName);
        } while (FindNextFile(hFind, &findFileData) != 0);

        FindClose(hFind);

        return fileNames;
    }
    std::vector<std::string> FileReadWriteLock::getFileNamesInDirectory(const std::string& directory, const std::string& fileEndig)
    {
        std::vector<std::string> fileNames;

        WIN32_FIND_DATA findFileData;
        HANDLE hFind = FindFirstFile((directory + "\\*" + fileEndig).c_str(), &findFileData);

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
            fileNames.push_back(findFileData.cFileName);
        } while (FindNextFile(hFind, &findFileData) != 0);

        FindClose(hFind);

        return fileNames;
    }
    const std::string& FileReadWriteLock::accessTypeToString(Access access)
    {
        switch (access)
        {
        case Access::read: { static const std::string msg = "r"; return msg; };
        case Access::write: { static const std::string msg = "w"; return msg; };
        case Access::readWrite: { static const std::string msg = "rw"; return msg; }
        }
        static const std::string unknown = "";
        return unknown;
    }
    FileReadWriteLock::Access FileReadWriteLock::stringToAccessType(const std::string& accessStr)
    {
        if (accessStr == "r")
            return Access::read;
        if (accessStr == "w")
            return Access::write;
        if (accessStr == "rw")
            return Access::readWrite;
        static Access unknown = Access::unknown;
        return unknown;
    }
    std::string FileReadWriteLock::getRandomString(size_t length)
    {
        static std::mt19937 generator(std::random_device{}());
        static const char charset[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";

        static std::uniform_int_distribution<int> distribution(0, sizeof(charset) - 2);
        static std::mutex mutex;

        std::string result;
        result.reserve(length);

        std::unique_lock<std::mutex> lock(mutex); // Lock to ensure thread safety

        for (size_t i = 0; i < length; ++i)
        {
            result += charset[distribution(generator)];
        }

        return result;
    }
}