#pragma once

#include "JD_base.h"
#include "JDDeclaration.h"
#include "utilities/filesystem/FileReadWriteLock.h"
#include "utilities/filesystem/FileChangeWatcher.h"
#include "utilities/filesystem/LockedFileAccessor.h"


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
                const std::string& databasePath,
                const std::string& databaseName,
                JDManager& manager,
                std::mutex &mtx);
            ~JDManagerFileSystem();
            bool setup();
        public:

            void setDatabasePath(const std::string& path);
            void setDatabaseName(const std::string& name);

            const std::string& getDatabaseName() const;
            const std::string& getDatabaseFileName() const;
            std::string getDatabasePath() const;

            std::string getDatabaseFilePath() const;

            static const std::string& getJsonFileEnding();
        protected:
            
            void logOnDatabase(std::string &generatedSessionIDOut);
            void logOffDatabase();
            bool isLoggedOnDatabase() const;



            bool makeDatabaseDirs() const;
            bool makeDatabaseFiles() const;
            bool deleteDir(const std::string& dir) const;
            bool deleteFile(const std::string& file) const;

            // Returns the amount of locks it has deleted
            int tryToClearUnusedFileLocks() const;

            ManagedFileChangeWatcher& getDatabaseFileWatcher();
            void restartDatabaseFileWatcher();
            class FileWatcherAutoPause
            {
            public:
                FileWatcherAutoPause(ManagedFileChangeWatcher& fs)
                    : m_fs(fs)
                { m_fs.pause(); }
                ~FileWatcherAutoPause()
                { m_fs.unpause(); }
            private:
                ManagedFileChangeWatcher& m_fs;
            };

            void update();
        private:
            std::string m_databasePath;
            std::string m_databaseName;
            std::string m_databaseFileName;

            size_t m_slowUpdateCounter;

            JDManager& m_manager;
            std::mutex& m_mutex;

            mutable FileReadWriteLock* m_fileLock;
            mutable ManagedFileChangeWatcher m_fileWatcher;

            // This lock file is used to ckeck if an user is still online or not.
            // If it can be deleted, the user is offline and did not clean up after himself.
            FileLock *m_databaseLoginFileLock;

            static const std::string s_jsonFileEnding;

        };
    }
}