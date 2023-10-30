#include "FileLock.h"

namespace JsonDatabase
{
    FileLock::FileLock(const std::string& filePath)
        : m_filePath(filePath) 
        , m_locked(false)
        , m_lastError(Error::none)
#ifdef _WIN32
        , m_fileHandle(nullptr)
#else
        , m_fileDescriptor(0);
#endif
    {

    }
    FileLock::~FileLock() 
    {
        unlockFile();
    }


    bool FileLock::lock()
    {
        m_lastError = Error::none;
        if (!m_locked)
        {
            m_lastError = lockFile();
        }
        JD_PROFILING_VALUE("locked", m_locked);
        if (m_lastError == Error::none)
            return true;
        return false;
    }
    void FileLock::unlock()
    {
        m_lastError = Error::none;
        unlockFile();
        JD_PROFILING_VALUE("locked", m_locked);
    }

    bool FileLock::isLocked() const
    {
        return m_locked;
    }
    enum FileLock::Error FileLock::getLastError() const
    {
        return m_lastError;
    }

    FileLock::Error FileLock::lockFile() 
    {
        JD_PROFILING_FUNCTION_C(profiler::colors::Red100);
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
            return Error::unableToCreateOrOpenLockFile;
            //throw std::runtime_error("Unable to create or open file.");
        }

        if (LockFile(m_fileHandle, 0, 0, MAXDWORD, MAXDWORD)) {
            //std::cout << "File locked successfully." << std::endl;
        }
        else {
            return Error::unableToLock;
            //throw std::runtime_error("Unable to lock file.");
        }
#else
        fileDescriptor = open(filePath.c_str(), O_RDWR | O_CREAT, 0666);

        if (fileDescriptor == -1) {
            return Error::unableToCreateOrOpenLockFile;
            //throw std::runtime_error("Unable to create or open file.");
        }

        struct flock fl;
        fl.l_type = F_WRLCK;
        fl.l_whence = SEEK_SET;
        fl.l_start = 0;
        fl.l_len = 0;

        if (fcntl(fileDescriptor, F_SETLKW, &fl) == -1) {
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
#ifdef _WIN32
        if (!m_fileHandle) return;

        UnlockFile(m_fileHandle, 0, 0, MAXDWORD, MAXDWORD);
        CloseHandle(m_fileHandle);
        m_fileHandle = nullptr;
#else
        if (!fileDescriptor) return;
        struct flock fl;
        fl.l_type = F_UNLCK;
        fl.l_whence = SEEK_SET;
        fl.l_start = 0;
        fl.l_len = 0;

        fcntl(fileDescriptor, F_SETLKW, &fl);
        close(fileDescriptor);
        fileDescriptor = 0;
#endif
        m_locked = false;

        //std::cout << "File unlocked successfully." << std::endl;
    }
}