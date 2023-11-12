#pragma once

#include "JD_base.h"
#include "JDDeclaration.h"
#include "utilities/filesystem/FileReadWriteLock.h"

#include <QJsonObject>
#include <mutex>

namespace JsonDatabase
{
    namespace Internal
    {
        class JSONDATABASE_EXPORT JDManagerFileSystem
        {
            friend class JDManager;
            JDManagerFileSystem(
                JDManager& manager,
                std::mutex &mtx);
            ~JDManagerFileSystem();
            void setup();
        public:

        protected:


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
            FileLock::Error getLastLockError() const;
            const std::string& getLastLockErrorStr() const;

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

            void restartFileWatcher();
            bool isFileWatcherRunning() const;
            void stopFileWatcher();
            bool fileWatcherHasFileChanged() const;
            void clearFileWatcherHasFileChanged();
            void pauseFileWatcher();
            void unpauseFileWatcher();
            bool isFileWatcherPaused() const;
        private:
            JDManager& m_manager;
            std::mutex& m_mutex;

            mutable FileReadWriteLock* m_fileLock;
            FileChangeWatcher* m_databaseFileWatcher;

            static const std::string s_jsonFileEnding;

        };
    }
}