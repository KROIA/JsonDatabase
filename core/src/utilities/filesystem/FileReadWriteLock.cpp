#include "utilities/filesystem/FileReadWriteLock.h"
#include "utilities/StringUtilities.h"

#include <random>

namespace JsonDatabase
{
    namespace Internal
    {
        const unsigned int FileReadWriteLock::s_tryLockTimeoutMs = 1000;


        FileReadWriteLock::FileReadWriteLock(const std::string& filePath, const std::string& fileName)
            : m_directory(Utilities::replaceForwardSlashesWithBackslashes(filePath))
            , m_fileName(fileName)
            , m_locked(false)
            , m_access(Access::unknown)
            , m_lock(nullptr)
        {
            // m_filePath = m_directory + "\\" + m_fileName;
        }
        FileReadWriteLock::~FileReadWriteLock()
        {
            FileLock::Error err;
            unlock(err);
        }

        const std::string& FileReadWriteLock::getFilePath() const
        {
            return m_directory;
        }
        const std::string& FileReadWriteLock::getFileName() const
        {
            return m_fileName;
        }
        bool FileReadWriteLock::lock(Access direction, bool& wasLockedByOtherUserOut, FileLock::Error& err)
        {
            wasLockedByOtherUserOut = false;
            err = lock_internal(direction, wasLockedByOtherUserOut);
#ifdef JD_DEBUG
            if (err != FileLock::Error::none)
            {
                JD_CONSOLE("bool FileReadWriteLock::lock(direction=" << accessTypeToString(direction) << ") " << FileLock::getErrorStr(err) + "\n");
            }
#endif
            return err == FileLock::Error::none;
        }
        bool FileReadWriteLock::lock(Access direction, unsigned int timeoutMs, bool& wasLockedByOtherUserOut, FileLock::Error& err)
        {
            if (m_locked)
            {
                err = FileLock::Error::alreadyLocked;
                return true;
            }
            err = FileLock::Error::none;

            // Get the current time
            auto start = std::chrono::high_resolution_clock::now();

            // Calculate the time point when the desired duration will be reached
            auto end = start + std::chrono::milliseconds(timeoutMs);
            wasLockedByOtherUserOut = false;
            bool timeout = false;
            while ((timeout = (std::chrono::high_resolution_clock::now() < end)) &&
                (err = lock_internal(direction, wasLockedByOtherUserOut)) != FileLock::Error::none)
            {
                JDFILE_FILE_LOCK_PROFILING_BLOCK("FileReadWriteLock::WaitForFreeLock", JD_COLOR_STAGE_5);
                // Sleep for a short while to avoid busy-waiting
                std::this_thread::yield();
            }
            if (timeout && err != FileLock::Error::none)
            {
                err = FileLock::Error::lockTimeout;
            }
#ifdef JD_DEBUG
            if (err != FileLock::Error::none)
            {
                JD_CONSOLE("bool FileReadWriteLock::lock(direction=" << accessTypeToString(direction) << ") " << FileLock::getErrorStr(err) + "\n");
            }
#endif
            return m_locked;
        }
        FileLock::Error FileReadWriteLock::lock_internal(Access direction, bool& wasLockedByOtherUserOut)
        {
            //JDFILE_FILE_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_7);
            FileLock::Error err = lockFile(direction, wasLockedByOtherUserOut);


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
            return err;
        }
        FileLock::Error FileReadWriteLock::lockFile(Access direction, bool& wasLockedByOtherUserOut)
        {
            if (m_lock)
                return FileLock::Error::alreadyLocked;
            FileLock::Error lockErr1;
            FileLock tmpLock(m_directory, m_fileName);
            if (!tmpLock.lock(s_tryLockTimeoutMs, lockErr1))
                return lockErr1;

            m_access = Access::unknown;

            size_t readerCount = 0;
            switch (getAccessStatus(readerCount))
            {
            case Access::readWrite:
            case Access::write:
            {
                wasLockedByOtherUserOut = true;
                // Some are writing, can't read or write
                return FileLock::Error::alreadyLockedForWritingByOther;
            }
            }

            if (direction == Access::write && readerCount != 0)
            {
                // Some are reading, can't write
                return FileLock::Error::alreadyLockedForReading;
            }


            std::string accessType = accessTypeToString(direction);
            std::string randStr;
            if (direction == Access::read)
                randStr = std::to_string(readerCount) + getRandomString(10);

            std::string lockFileName = m_fileName + "_" + accessType + "-" + randStr;

            FileLock::Error lockErr2;
            m_lock = new FileLock(m_directory, lockFileName);
            if (!m_lock->lock(lockErr2))
            {
                delete m_lock;
                m_lock = nullptr;
                return lockErr2;
            }

            m_locked = true;
            m_access = direction;
            m_lockFilePathName = lockFileName;
            return FileLock::Error::none;
        }
        void FileReadWriteLock::unlock(FileLock::Error& err)
        {
#ifdef JD_PROFILING
            //JDFILE_FILE_LOCK_PROFILING_FUNCTION(JD_COLOR_STAGE_7);
            bool wasLocked = m_locked;
#endif
            if (m_lock)
            {
                m_locked = false;
                m_lock->unlock(err);
                delete m_lock;
                m_lock = nullptr;
            }
            else
            {
                err = FileLock::alreadyUnlocked;
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
        FileReadWriteLock::Access FileReadWriteLock::getAccessStatus() const
        {
            size_t dummy;
            return getAccessStatus(dummy);
        }
        FileReadWriteLock::Access FileReadWriteLock::getAccessStatus(size_t& readerCount) const
        {
            std::vector<std::string> files = FileLock::getFileNamesInDirectory(m_directory, FileLock::s_lockFileEnding);
            readerCount = 0;
            for (const std::string& file : files)
            {
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
                {
                    return Access::readWrite;
                }
                case Access::write:
                {
                    return Access::write;
                }
                case Access::read:
                {
                    ++readerCount;
                    break;
                }
                }
            }
            if (readerCount > 0)
                return Access::read;
            return Access::unknown;
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
}