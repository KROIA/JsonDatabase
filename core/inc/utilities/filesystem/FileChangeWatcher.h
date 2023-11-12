#pragma once

#include "JD_base.h"
#include "Signal.h"

#include <windows.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace JsonDatabase
{
    namespace Internal
    {
        class JSONDATABASE_EXPORT FileChangeWatcher
        {
        public:
            FileChangeWatcher(const std::string& filePath);

            ~FileChangeWatcher();

            void startWatching();
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
            std::thread *m_watchThread;
            std::mutex m_mutex;
            std::condition_variable m_cv;
            FILETIME m_lastModificationTime;
            std::atomic<bool> m_stopFlag;
            std::atomic<bool> m_fileChanged;
            std::atomic<bool> m_paused;
        };
    }
}

