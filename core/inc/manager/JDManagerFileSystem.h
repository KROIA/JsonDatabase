#pragma once

#include "JD_base.h"
#include "JDDeclaration.h"
#include "utilities/filesystem/FileReadWriteLock.h"
#include "utilities/filesystem/FileChangeWatcher.h"

#ifdef JD_USE_QJSON
#include <QJsonObject>
#else
#include "Json/JsonValue.h"
#endif
#include <mutex>

namespace JsonDatabase
{
    namespace Internal
    {
        class JSONDATABASE_EXPORT JDManagerFileSystem
        {
        protected:
            JDManagerFileSystem(
                JDManager& manager,
                std::mutex &mtx);
            ~JDManagerFileSystem();
            bool setup();
        public:
            enum class Error
            {
                none = 0,

                // File locking
                fileLock_unableToCreateOrOpenLockFile = 1,
                fileLock_unableToDeleteLockFile = 2,
                fileLock_unableToLock = 3,
                fileLock_alreadyLocked = 4,
                fileLock_alreadyLockedForReading = 5,
                fileLock_alreadyLockedForWriting = 6,
                fileLock_alreadyUnlocked = 7,

                // Json reading/writing
                json_parseError = 30,

                // File reading/writing
                file_cantOpenFileForRead = 40,
                file_cantOpenFileForWrite = 41,
                file_invalidFileSize = 42,
                file_cantReadFile = 43,
                file_cantWriteFile = 44,
                file_cantVerifyFileContents = 45,
            };

        protected:
            static const std::string& getJsonFileEnding();

            bool lockFile(
                const std::string& directory,
                const std::string& fileName,
                FileReadWriteLock::Access direction,
                bool & wasLockedForWritingByOther,
                Error &errorOut) const;
            bool lockFile(
                const std::string& directory,
                const std::string& fileName,
                FileReadWriteLock::Access direction,
                bool& wasLockedForWritingByOther,
                unsigned int timeoutMillis,
                Error& errorOut) const;
            bool unlockFile(Error& errorOut) const;
            bool isFileLockedByOther(
                const std::string& directory,
                const std::string& fileName,
                FileReadWriteLock::Access accessType) const;
            
#ifdef JD_USE_QJSON
            bool writeJsonFile(
                const std::vector<QJsonObject>& jsons,
                const std::string& directory,
                const std::string& fileName,
                const std::string& fileEnding,
                bool zipFormat,
                bool lockedRead,
                Error& errorOut,
                Internal::WorkProgress* progress) const;
            bool writeJsonFile(
                const QJsonObject& json,
                const std::string& directory,
                const std::string& fileName,
                const std::string& fileEnding,
                bool zipFormat,
                bool lockedRead,
                Error& errorOut,
                Internal::WorkProgress* progress) const;


            bool readJsonFile(
                std::vector<QJsonObject>& jsonsOut,
                const std::string& directory,
                const std::string& fileName,
                const std::string& fileEnding,
                bool zipFormat,
                bool lockedRead,
                Error& errorOut,
                Internal::WorkProgress* progress) const;
            bool readJsonFile(
                QJsonObject& objOut,
                const std::string& directory,
                const std::string& fileName,
                const std::string& fileEnding,
                bool zipFormat,
                bool lockedRead,
                Error& errorOut,
                Internal::WorkProgress* progress) const;
            /*
            bool readFile(
                QByteArray& fileDataOut,
                const std::string& directory,
                const std::string& fileName,
                const std::string& fileEnding,
                bool lockedRead) const;
            bool writeFile(
                const QByteArray& fileData,
                const std::string& directory,
                const std::string& fileName,
                const std::string& fileEnding,
                bool lockedRead) const;
                */

#else
            bool writeJsonFile(
                const JsonArray& jsons,
                const std::string& directory,
                const std::string& fileName,
                const std::string& fileEnding,
                bool zipFormat,
                bool lockedRead,
                Error& errorOut,
                Internal::WorkProgress* progress) const;
            bool writeJsonFile(
                const JsonValue& json,
                const std::string& directory,
                const std::string& fileName,
                const std::string& fileEnding,
                bool zipFormat,
                bool lockedRead,
                Error& errorOut,
                Internal::WorkProgress* progress) const;


            bool readJsonFile(
                JsonArray& jsonsOut,
                const std::string& directory,
                const std::string& fileName,
                const std::string& fileEnding,
                bool zipFormat,
                bool lockedRead,
                Error& errorOut,
                Internal::WorkProgress* progress) const;
            bool readJsonFile(
                JsonValue& objOut,
                const std::string& directory,
                const std::string& fileName,
                const std::string& fileEnding,
                bool zipFormat,
                bool lockedRead,
                Error& errorOut,
                Internal::WorkProgress* progress) const;

           /* bool readFile(
                std::string& fileDataOut,
                const std::string& directory,
                const std::string& fileName,
                const std::string& fileEnding,
                bool lockedRead) const;
            bool writeFile(
                const std::string& fileData,
                const std::string& directory,
                const std::string& fileName,
                const std::string& fileEnding,
                bool lockedRead) const;*/
#endif
            bool readFile(
                QByteArray& fileDataOut,
                const std::string& directory,
                const std::string& fileName,
                const std::string& fileEnding,
                bool lockedRead,
                Error& errorOut,
                Internal::WorkProgress* progress) const;
            bool writeFile(
                const QByteArray& fileData,
                const std::string& directory,
                const std::string& fileName,
                const std::string& fileEnding,
                bool lockedRead,
                Error& errorOut,
                Internal::WorkProgress* progress) const;

            bool makeDatabaseDirs() const;
            bool makeDatabaseFiles() const;
            bool deleteDir(const std::string& dir) const;
            bool deleteFile(const std::string& file) const;

            // Returns the amount of locks it has deleted
            int tryToClearUnusedFileLocks() const;

            ManagedFileChangeWatcher& getDatabaseFileWatcher();
            void restartDatabaseFileWatcher();

            void update();

            const std::string& getErrorStr(Error err);
        private:
            size_t m_slowUpdateCounter;

            JDManager& m_manager;
            std::mutex& m_mutex;

            mutable FileReadWriteLock* m_fileLock;
            mutable ManagedFileChangeWatcher m_fileWatcher;

            static const std::string s_jsonFileEnding;

        };
    }
}