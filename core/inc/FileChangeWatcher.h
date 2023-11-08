#pragma once

#include "JD_base.h"
#include "Signal.h"

#include <windows.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace JsonDatabase
{

    class JSONDATABASE_EXPORT FileChangeWatcher 
    {
    public:
        FileChangeWatcher(const std::string& filePath);

        ~FileChangeWatcher();

        void startWatching();
        void stopWatching();
        bool hasFileChanged();
        void clearFileChangedFlag();

        void pause();
        void unpause();

        //void connectSlot(const Signal::SlotFunction& slotFunction);
        //void disconnectSlot(const Signal::SlotFunction& slotFunction);

    private:
        std::string getFullPath(const std::string& relativePath);
        void monitorFileChanges();
        bool fileChanged();

        std::string m_filePath;
        HANDLE m_eventHandle;
        std::thread m_watchThread;
        std::mutex m_mutex;
        std::condition_variable m_cv;
        FILETIME m_lastModificationTime;
        bool m_stopFlag;
        bool m_fileChanged;
        bool m_paused;

        //Signal m_fileChangedSignal;
    };
}

