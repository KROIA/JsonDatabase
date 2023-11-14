#include "utilities/filesystem/FileChangeWatcher.h"
#include "utilities/JDUtilities.h"

namespace JsonDatabase
{
    namespace Internal
    {
        FileChangeWatcher::FileChangeWatcher(const std::string& filePath)
            : m_stopFlag(false)
            , m_fileChanged(false)
            , m_paused(false)
            , m_watchThread(nullptr)
        {
            m_filePath = getFullPath(filePath);
            std::string directory = m_filePath.substr(0, m_filePath.find_last_of("\\") + 1);
            m_eventHandle = FindFirstChangeNotificationA(directory.c_str(), FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE);
            if (m_eventHandle == INVALID_HANDLE_VALUE) 
            {
                DWORD error = GetLastError();
                JD_CONSOLE_FUNCTION("Error initializing file change monitoring. GetLastError() =  " << error << " : "<< Utilities::getLastErrorString(error) << "\n");
            }
        }

        FileChangeWatcher::~FileChangeWatcher()
        {
            FindCloseChangeNotification(m_eventHandle);
            stopWatching();
        }

        void FileChangeWatcher::startWatching()
        {
            if (m_watchThread)
                return;
            m_watchThread = new std::thread(&FileChangeWatcher::monitorFileChanges, this);
        }

        void FileChangeWatcher::stopWatching()
        {
            if (!m_watchThread)
                return;
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_stopFlag.store(true);
                m_cv.notify_all();
            }
            m_watchThread->join();
            delete m_watchThread;
            m_watchThread = nullptr;
        }
        bool FileChangeWatcher::isWatching() const
        {
            return m_watchThread;
        }

        bool FileChangeWatcher::hasFileChanged()
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            return m_fileChanged;
        }

        void FileChangeWatcher::clearFileChangedFlag()
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_fileChanged.store(false);
            m_cv.notify_all();
        }

        void FileChangeWatcher::pause()
        {
            m_paused.store(true);
        }
        void FileChangeWatcher::unpause()
        {
            m_paused.store(false);
        }
        bool FileChangeWatcher::isPaused() const
        {
            return m_paused.load();
        }

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
            BOOL success = ReadDirectoryChangesW(
                m_eventHandle,
                buffer,
                sizeof(buffer),
                TRUE,
                FILE_NOTIFY_CHANGE_LAST_WRITE,
                &bytesReturned,
                nullptr,
                nullptr
            );
            while (!m_stopFlag) {

                DWORD waitResult = WAIT_FAILED;
                while (waitResult != WAIT_OBJECT_0)
                {
                    waitResult = WaitForSingleObject(m_eventHandle, 100);
                    if (m_stopFlag)
                        break;
                }
                if (waitResult == WAIT_OBJECT_0) {


                    if (fileChanged() && !m_paused.load())
                    {
                        std::unique_lock<std::mutex> lock(m_mutex);

                        m_fileChanged.store(true);
                        while (m_fileChanged && !m_stopFlag.load()) {
                            m_cv.wait(lock);
                        }

                        if (m_stopFlag.load()) {
                            break;
                        }
                    }

                    ResetEvent(m_eventHandle);

                    
                    
                    //m_eventHandle = FindFirstChangeNotificationA(m_filePath.c_str(), FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE);

                    if (!success) {
                        DWORD error = GetLastError();
                        JD_CONSOLE_FUNCTION("Error monitoring file changes. GetLastError() =  " << error << " : " << Utilities::getLastErrorString(error) << "\n");
                    }
                }
                else {
                    DWORD error = GetLastError();
                    JD_CONSOLE_FUNCTION("Error waiting for file changes. GetLastError() =  " << error << " : " << Utilities::getLastErrorString(error) << "\n");
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




        //
        //    ManagerdFileChangeWatcher
        //


        ManagedFileChangeWatcher::ManagedFileChangeWatcher()
            : m_databaseFileWatcher(nullptr)
        {

        }
        ManagedFileChangeWatcher::~ManagedFileChangeWatcher()
        {
            if (m_databaseFileWatcher)
            {
                m_databaseFileWatcher->stopWatching();
                delete m_databaseFileWatcher;
            }
        }
        void ManagedFileChangeWatcher::setup(const std::string& targetFile)
        {
            restart(targetFile);
        }
        void ManagedFileChangeWatcher::restart(const std::string& targetFile)
        {
            if (m_databaseFileWatcher)
            {
                m_databaseFileWatcher->stopWatching();
                delete m_databaseFileWatcher;
                m_databaseFileWatcher = nullptr;
            }
            m_databaseFileWatcher = new FileChangeWatcher(targetFile);
            m_databaseFileWatcher->startWatching();
        }

        bool ManagedFileChangeWatcher::isRunning() const
        {
            if (!m_databaseFileWatcher)
                return false;
            return m_databaseFileWatcher->isWatching();
        }
        void ManagedFileChangeWatcher::stop()
        {
            if (!m_databaseFileWatcher)
                return;
            m_databaseFileWatcher->stopWatching();
        }
        bool ManagedFileChangeWatcher::hasFileChanged() const
        {
            if (!m_databaseFileWatcher)
                return false;
            return m_databaseFileWatcher->hasFileChanged();
        }
        void ManagedFileChangeWatcher::clearHasFileChanged()
        {
            if (!m_databaseFileWatcher)
                return;
            m_databaseFileWatcher->clearFileChangedFlag();
        }

        void ManagedFileChangeWatcher::pause()
        {
            if (!m_databaseFileWatcher)
                return;
            m_databaseFileWatcher->pause();
        }
        void ManagedFileChangeWatcher::unpause()
        {
            if (!m_databaseFileWatcher)
                return;
            m_databaseFileWatcher->unpause();
        }
        bool ManagedFileChangeWatcher::isPaused() const
        {
            if (!m_databaseFileWatcher)
                return false;
            return m_databaseFileWatcher->isPaused();
        }
    }
}