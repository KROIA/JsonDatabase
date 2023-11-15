#pragma once

#include "JD_base.h"
#include "JDDeclaration.h"
#include "utilities/filesystem/FileReadWriteLock.h"
#include "utilities/filesystem/FileChangeWatcher.h"

#include <QJsonObject>
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

        protected:
            static const std::string& getJsonFileEnding();

            bool lockFile(
                const std::string& directory,
                const std::string& fileName,
                FileReadWriteLock::Access direction,
                bool & wasLockedForWritingByOther) const;
            bool lockFile(
                const std::string& directory,
                const std::string& fileName,
                FileReadWriteLock::Access direction,
                bool& wasLockedForWritingByOther,
                unsigned int timeoutMillis) const;
            bool unlockFile() const;
            bool isFileLockedByOther(const
                std::string& directory,
                const std::string& fileName,
                FileReadWriteLock::Access accessType) const;
            
            bool writeJsonFile(
                const std::vector<QJsonObject>& jsons,
                const std::string& directory,
                const std::string& fileName,
                const std::string& fileEnding,
                bool zipFormat,
                bool lockedRead) const;
            bool writeJsonFile(
                const QJsonObject& json,
                const std::string& directory,
                const std::string& fileName,
                const std::string& fileEnding,
                bool zipFormat,
                bool lockedRead) const;

            bool readJsonFile(
                std::vector<QJsonObject>& jsonsOut,
                const std::string& directory,
                const std::string& fileName,
                const std::string& fileEnding,
                bool zipFormat,
                bool lockedRead) const;
            bool readJsonFile(
                QJsonObject& objOut,
                const std::string& directory,
                const std::string& fileName,
                const std::string& fileEnding,
                bool zipFormat,
                bool lockedRead) const;

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

            bool makeDatabaseDirs() const;
            bool makeDatabaseFiles() const;
            bool deleteDir(const std::string& dir) const;
            bool deleteFile(const std::string& file) const;

            ManagedFileChangeWatcher& getDatabaseFileWatcher();
            void restartDatabaseFileWatcher();

            void update();

            
        private:
            JDManager& m_manager;
            std::mutex& m_mutex;

            mutable FileReadWriteLock* m_fileLock;
            mutable ManagedFileChangeWatcher m_fileWatcher;

            static const std::string s_jsonFileEnding;

        };
    }
}