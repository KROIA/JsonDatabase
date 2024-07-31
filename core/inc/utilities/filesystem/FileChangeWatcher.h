#pragma once

#include "JsonDatabase_base.h"
#include "Signal.h"
#include "JsonDatabase_Declaration.h"

#include <windows.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <filesystem>

#include "Logger.h"

#if !defined(JD_FILEWATCHER_USE_POLLING) and !defined(JD_FILEWATCHER_USE_WIN_API)
    #error "Please define either JD_FILEWATCHER_USE_POLLING or JD_FILEWATCHER_USE_WIN_API in JsonDatabase_global.h"
#else 
#if defined(JD_FILEWATCHER_USE_POLLING) and defined(JD_FILEWATCHER_USE_WIN_API)
	#error "Please define either JD_FILEWATCHER_USE_POLLING or JD_FILEWATCHER_USE_WIN_API in JsonDatabase_global.h"
#endif
#endif



namespace JsonDatabase
{
    namespace Internal
    {
        class JSON_DATABASE_EXPORT FileChangeWatcher
        {
        public:
            FileChangeWatcher(const std::string& filePath);
            ~FileChangeWatcher();
            bool setup(Log::LogObject* parentLogger);
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
            
            bool fileChanged();

            Log::LogObject* m_logger = nullptr;
            std::string m_filePath;

#ifdef JD_FILEWATCHER_USE_WIN_API
            void monitorFileChanges();
            std::thread* m_watchThread = nullptr;
            std::atomic<HANDLE> m_eventHandle;
#endif
#ifdef JD_FILEWATCHER_USE_POLLING
            void checkFileChanges();
            //std::string getMd5(const std::string& fullFilePath);
            std::string m_md5Hash;
#endif
            DWORD m_setupError;

            
            std::mutex m_mutex;
            std::condition_variable m_cv;
            std::filesystem::file_time_type m_lastModificationTime;
            //FILETIME m_lastModificationTime;
            std::atomic<bool> m_stopFlag;
            std::atomic<bool> m_fileChanged;
            std::atomic<bool> m_paused;
        };


        class JSON_DATABASE_EXPORT ManagedFileChangeWatcher
        {
            friend JDManagerFileSystem;
            friend JDObjectLocker;
            bool setup(const std::string& targetFile, Log::LogObject* parentLogger);
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
            
            Log::LogObject* m_logger = nullptr;
            FileChangeWatcher* m_databaseFileWatcher;
        };
    }
}

