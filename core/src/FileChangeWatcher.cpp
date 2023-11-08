#include "FileChangeWatcher.h"


namespace JsonDatabase
{

    FileChangeWatcher::FileChangeWatcher(const std::string& filePath)
        : m_stopFlag(false)
        , m_fileChanged(false)
        , m_paused(false)
    {
        //std::wstring wsTmp(filePath.begin(), filePath.end());
        //m_filePath = wsTmp;

        m_filePath = getFullPath(filePath);
        //std::cout << "Full path: "<<m_filePath << std::endl;
        std::string directory = m_filePath.substr(0, m_filePath.find_last_of("\\") + 1);
        m_eventHandle = FindFirstChangeNotificationA(directory.c_str(), FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE);
        if (m_eventHandle == INVALID_HANDLE_VALUE) {
            DWORD error = GetLastError();
            JD_CONSOLE_FUNCTION("Error initializing file change monitoring. GetLastError() =  " << error << "\n");

            //throw std::runtime_error("Error initializing file change monitoring.");
        }
    }

    FileChangeWatcher::~FileChangeWatcher() 
    {
        FindCloseChangeNotification(m_eventHandle);
    }

    void FileChangeWatcher::startWatching()
    {
        m_watchThread = std::thread(&FileChangeWatcher::monitorFileChanges, this);
    }

    void FileChangeWatcher::stopWatching()
    {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_stopFlag = true;
            m_cv.notify_all();
        }
        m_watchThread.join();
    }

    bool FileChangeWatcher::hasFileChanged() 
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_fileChanged;
    }

    void FileChangeWatcher::clearFileChangedFlag()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_fileChanged = false;
        m_cv.notify_all();
    }

    void FileChangeWatcher::pause()
    {
        m_paused = true;
    }
    void FileChangeWatcher::unpause()
    {
        m_paused = false;
    }


    /*void FileChangeWatcher::connectSlot(const Signal::SlotFunction& slotFunction)
    {
        m_fileChangedSignal.connect(slotFunction);
    }*/

    std::string FileChangeWatcher::getFullPath(const std::string& relativePath) {
        char fullPath[MAX_PATH];
        DWORD result = GetFullPathNameA(relativePath.c_str(), MAX_PATH, fullPath, nullptr);

        if (result == 0) {
            throw std::runtime_error("Error getting full path.");
        }

        return fullPath;
    }

    
    void FileChangeWatcher::monitorFileChanges() 
    {
        DWORD bytesReturned;
        BYTE buffer[4096];

        fileChanged(); // Set initial file modification time
        while (!m_stopFlag) {

            DWORD waitResult = WAIT_FAILED;
            while (waitResult != WAIT_OBJECT_0)
            {
                waitResult = WaitForSingleObject(m_eventHandle, 100);
                if (m_stopFlag)
                    break;
            }
            if (waitResult == WAIT_OBJECT_0) {
                

                if (fileChanged() && !m_paused)
                {
                    std::unique_lock<std::mutex> lock(m_mutex);

                   // JD_GENERAL_PROFILING_BLOCK("File change detected", JD_COLOR_STAGE_5);
                    m_fileChanged = true;
                    ResetEvent(m_eventHandle);


                    while (m_fileChanged && !m_stopFlag) {
                        m_cv.wait(lock);
                    }

                    if (m_stopFlag) {
                        break;
                    }
                }

                

                BOOL success = ReadDirectoryChangesW(
                    FindFirstChangeNotificationA(m_filePath.c_str(), FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE),
                    buffer,
                    sizeof(buffer),
                    TRUE,
                    FILE_NOTIFY_CHANGE_LAST_WRITE,
                    &bytesReturned,
                    nullptr,
                    nullptr
                );

                if (!success) {
                    //throw std::runtime_error("Error monitoring file changes.");
                    DWORD error = GetLastError();
                    JD_CONSOLE_FUNCTION("Error monitoring file changes. GetLastError() =  " << error << "\n");
                }
            }
            else {
                DWORD error = GetLastError();
                JD_CONSOLE_FUNCTION("Error waiting for file changes. GetLastError() =  " << error << "\n");
                //throw std::runtime_error("Error waiting for file changes.");
            }
        }
    }
    bool FileChangeWatcher::fileChanged()
    {
        WIN32_FILE_ATTRIBUTE_DATA fileData;
        if (!GetFileAttributesEx(m_filePath.c_str(), GetFileExInfoStandard, &fileData)) {
            return false;
        }

        if (CompareFileTime(&fileData.ftLastWriteTime, &m_lastModificationTime) != 0) {
            m_lastModificationTime = fileData.ftLastWriteTime;
            return true; // File has changed
        }
        return false;
    }
}