#pragma once

#include "JsonDatabase_base.h"
#include "JsonDatabase_Declaration.h"
#include "utilities/filesystem/FileReadWriteLock.h"
#include "utilities/filesystem/FileChangeWatcher.h"
#include "utilities/filesystem/LockedFileAccessor.h"
#include "utilities/JDUserRegistration.h"

#include "Logger.h"

#include "Json/JsonValue.h"
#include <mutex>

namespace JsonDatabase
{
    namespace Internal
    {
        class JSON_DATABASE_EXPORT JDManagerFileSystem
        {
        protected:
            JDManagerFileSystem(
                const std::string& databasePath,
                const std::string& databaseName,
                JDManager& manager,
                std::mutex &mtx);
            ~JDManagerFileSystem();
            void setParentLogger(Log::Logger::ContextLogger* parentLogger);
            bool setup();
        public:

            void setDatabasePath(const std::string& path);
            void setDatabaseName(const std::string& name);

            const std::string& getDatabaseName() const;
            const std::string& getDatabaseFileName() const;
            std::string getDatabasePath() const;

            std::string getDatabaseFilePath() const;

            bool isLoggedOnDatabase() const;


            static const std::string& getJsonFileEnding();
        protected:
            
            void logOnDatabase();
            void logOffDatabase();



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

            Utilities::JDUserRegistration m_userRegistration;
            // This lock file is used to ckeck if an user is still online or not.
            // If it can be deleted, the user is offline and did not clean up after himself.
            //FileLock *m_databaseLoginFileLock;

            Log::Logger::ContextLogger* m_logger = nullptr;

            static const std::string s_jsonFileEnding;

        };
    }
}
