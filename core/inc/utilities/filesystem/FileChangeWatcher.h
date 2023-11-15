#pragma once

#include "JD_base.h"
#include "Signal.h"
#include "JDDeclaration.h"

#include <windows.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <filesystem>

namespace JsonDatabase
{
    namespace Internal
    {
        class JSONDATABASE_EXPORT FileChangeWatcher
        {
        public:
            FileChangeWatcher(const std::string& filePath);
            ~FileChangeWatcher();
            bool setup();
            DWORD getSetupError() const;

            bool startWatching();
            void stopWatching();
            bool isWatching() const;
            bool hasFileChanged();
            void clearFileChangedFlag();

            void pause();
            void unpause();
            bool isPaused() const;

        private:
            std::string getFullPath(const std::string& relativePath);
            void monitorFileChanges();
            bool fileChanged();

            std::string m_filePath;
            HANDLE m_eventHandle;
            DWORD m_setupError;
            std::thread *m_watchThread;
            std::mutex m_mutex;
            std::condition_variable m_cv;
            std::filesystem::file_time_type m_lastModificationTime;
            //FILETIME m_lastModificationTime;
            std::atomic<bool> m_stopFlag;
            std::atomic<bool> m_fileChanged;
            std::atomic<bool> m_paused;
        };


        class JSONDATABASE_EXPORT ManagedFileChangeWatcher
        {
            friend JDManagerFileSystem;
            friend JDObjectLocker;
            bool setup(const std::string& targetFile);
            ManagedFileChangeWatcher();
            ~ManagedFileChangeWatcher();
        public:
            
            
            bool restart(const std::string& targetFile);
            bool isRunning() const;
            void stop();
            bool hasFileChanged() const;
            void clearHasFileChanged();
            void pause();
            void unpause();
            bool isPaused() const;

        private:
            FileChangeWatcher* m_databaseFileWatcher;
        };
    }
}

