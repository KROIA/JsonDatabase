#pragma once

#include "JD_base.h"

#include <string>
#include <vector>

#include <windows.h>
#include <mutex>



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
        


        FileLock(const std::string& filePath, const std::string &fileName);

        ~FileLock();

        const std::string& getFilePath() const;
        const std::string& getFileName() const;

        bool lock();
        bool lock(unsigned int timeoutMs);
        void unlock();

        bool isLocked() const;

        Error getLastError() const;
        const std::string& getLastErrorStr() const;


        static std::string replaceForwardSlashesWithBackslashes(const std::string& input);
        
        

        static const std::string s_lockFileEnding;
    private:
        bool lock_internal();
        Error lockFile();
        //Error lockFile_old();
        void unlockFile();

      


        std::string m_filePath;
        std::string m_directory;
        std::string m_fileName;

        std::string m_lockFilePathName;

        HANDLE m_fileHandle;
        
        

        bool m_locked;
        Error m_lastError;

        
        static std::mutex m_mutex;
    };
}