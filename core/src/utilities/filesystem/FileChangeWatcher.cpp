#include "utilities/filesystem/FileChangeWatcher.h"
#include "utilities/JDUtilities.h"
#include "utilities/StringUtilities.h"
#include <fstream>
#include <filesystem>


namespace JsonDatabase
{
    namespace Internal
    {
        FileChangeWatcher::Mode FileChangeWatcher::s_defaultWatchMode = FileChangeWatcher::Mode::polling;

        FileChangeWatcher::FileChangeWatcher(const std::string& filePath)
            : m_stopFlag(false)
            , m_fileChanged(false)
            , m_paused(false)
            , m_eventHandle(nullptr)
            , m_setupError(0)
        {
            m_filePath = getFullPath(filePath);
            m_watchMode = s_defaultWatchMode;
        }

        FileChangeWatcher::~FileChangeWatcher()
        {
            stopWatching();
        }

        bool FileChangeWatcher::setup(Log::LogObject* parentLogger)
        {
            std::string directory = m_filePath.substr(0, m_filePath.find_last_of("\\") + 1);
            /*m_eventHandle = FindFirstChangeNotificationA(directory.c_str(), FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE);
            if (m_eventHandle == INVALID_HANDLE_VALUE)
            {
                m_setupError = GetLastError();
                JD_CONSOLE_FUNCTION("Error initializing file change monitoring. GetLastError() =  " << m_setupError << " : " << Utilities::getLastErrorString(m_setupError) << "\n");
                return false;
            }*/
            m_logger = parentLogger;
            m_setupError = 0;
            return true;
        }
        DWORD FileChangeWatcher::getSetupError() const
        {
			return m_setupError;
		}

        bool FileChangeWatcher::startWatching(Mode watchMode)
        {
            m_watchMode = watchMode;
            switch(m_watchMode)
			{
                case Mode::polling:
                {
                    checkFileChanges();
                    break;
                }
                case Mode::winApi:
                {
                    if (m_watchThread)
                        return true;
                    if (!m_eventHandle.load())
                        if (!setup(m_logger))
                            return false;

                    m_watchThread = new std::thread(&FileChangeWatcher::monitorFileChanges, this);
                }
                default:
                {
                    return false;
                }
            }
            return true;
        }
        bool FileChangeWatcher::startWatching()
        {
            return startWatching(m_watchMode);
		}

        void FileChangeWatcher::stopWatching()
        {
            switch (m_watchMode)
            {
                case Mode::polling:
                {
                    
                    break;
                }
                case Mode::winApi:
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
            }
        }
        bool FileChangeWatcher::isWatching() const
        {
            switch (m_watchMode)
            {
                case Mode::polling:
                {
                    return true;
                    break;
                }
                case Mode::winApi:
                {
                    return m_watchThread != nullptr;
                }
            }
            return false;
        }

        bool FileChangeWatcher::hasFileChanged()
        {
            std::unique_lock<std::mutex> lock(m_mutex);

            if(m_watchMode == Mode::polling)
				checkFileChanges();

            return m_fileChanged;
        }

        void FileChangeWatcher::clearFileChangedFlag()
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_fileChanged.store(false);
            if(m_watchMode == Mode::winApi)
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


        void FileChangeWatcher::setDefaultWatchMode(Mode mode)
        {
            s_defaultWatchMode = mode;
        }
        FileChangeWatcher::Mode FileChangeWatcher::getDefaultWatchMode()
        {
            return s_defaultWatchMode;
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
            JD_PROFILING_THREAD("FileChangeWatcher");
#ifdef JD_PROFILING
            std::string title = ("FileChangeWatcher \"" + m_filePath + "\"");            
            JD_GENERAL_PROFILING_BLOCK(title.c_str(), JD_COLOR_STAGE_7);
#endif
            DWORD bytesReturned;
            BYTE buffer[4096];

            std::string directory = m_filePath.substr(0, m_filePath.find_last_of("\\") + 1);
#ifdef UNICODE
            m_eventHandle.store(FindFirstChangeNotification(Utilities::strToWstr(directory).c_str(), FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE));
#else
            m_eventHandle.store(FindFirstChangeNotification(directory.c_str(), FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE));
#endif

            if (m_eventHandle.load() == INVALID_HANDLE_VALUE) {
                if (m_logger)m_logger->logError("starting directory watch: " + Utilities::getLastErrorString(GetLastError()) + "\n");
                return;
            }

            fileChanged(); // Set initial file modification time
            while (!m_stopFlag.load()) {
                JD_GENERAL_PROFILING_BLOCK("while",JD_COLOR_STAGE_8);
                DWORD waitResult = WAIT_FAILED;
                {
                    JD_GENERAL_PROFILING_BLOCK("waitForChange", JD_COLOR_STAGE_9);
                    while (waitResult != WAIT_OBJECT_0)
                    {
                        waitResult = WaitForSingleObject(m_eventHandle.load(), 1000);
                        if (m_stopFlag.load())
                        {
                            waitResult = WAIT_FAILED;
                            goto exitThread;
                        }
                    }
                }
                if (waitResult == WAIT_OBJECT_0) {
                    JD_GENERAL_PROFILING_BLOCK("readFileChange", JD_COLOR_STAGE_9);
                    
                    HANDLE cHandle = m_eventHandle.load();
                    if(cHandle)
                    {
                        if (m_stopFlag.load())
                        {
							goto exitThread;
						}
                        bool res = FindNextChangeNotification(cHandle);
#ifdef JD_DEBUG
                        if (!res)
                        {
                            DWORD error = GetLastError();
                            if (m_logger)
                                m_logger->logError("FindNextChangeNotification. GetLastError() =  " + std::to_string(error) + " : " + Utilities::getLastErrorString(error));
                        }
#elif 
                        JD_UNUSED(res);
#endif
                    }

                    if (fileChanged() && !m_paused.load())
                    {
                        JD_GENERAL_PROFILING_BLOCK("Change detectd, waitForLockRelease", JD_COLOR_STAGE_9);
                        std::unique_lock<std::mutex> lock(m_mutex);

                        m_fileChanged.store(true);
                        if(m_logger)
                            m_logger->logInfo("File change detected");
                        while (m_fileChanged && !m_stopFlag.load()) {
                            m_cv.wait(lock);
                        }

                        if (m_stopFlag.load()) {
                            break;
                        }
                    }
                }
#ifdef JD_DEBUG
                else {
                    DWORD error = GetLastError();
                    if(m_logger)m_logger->logError("Waiting for file changes. GetLastError() =  " + std::to_string(error) + " : " + Utilities::getLastErrorString(error));
                }
#endif
            }
        exitThread:;

            FindCloseChangeNotification(m_eventHandle.load());
            m_eventHandle.store(nullptr);
        }


        bool FileChangeWatcher::fileChanged()
        {
            std::filesystem::path file(m_filePath);

            if (!std::filesystem::exists(file)) {
                if(m_logger)m_logger->logError("File: \"" + m_filePath + "\" does not exist!");
                return false;
            }
            // Wait for a short duration to ensure any ongoing file operation is completed
            // std::this_thread::sleep_for(std::chrono::milliseconds(100));

            // Get the last modification time of the file
            std::filesystem::file_time_type change = std::filesystem::last_write_time(file);


            // Check if the file has been modified since lastWriteTime
            if (change > m_lastModificationTime) {
                HANDLE fileHandle = CreateFile(
#ifdef UNICODE
                    Utilities::strToWstr(m_filePath).c_str(),
#else
                    m_filePath.c_str(),
#endif                     
                    GENERIC_READ,
                    FILE_SHARE_READ,
                    nullptr,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    nullptr
                );

                if (fileHandle == INVALID_HANDLE_VALUE) 
                {
                    // pause for 1 ms
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    return false;
                }
                m_lastModificationTime = change;
                // Close the file handle
                CloseHandle(fileHandle);
                return true;
            }

            return false; // File has not changed
        }
        void FileChangeWatcher::checkFileChanges()
        {
            bool success;
            std::string md5 = Utilities::calculateMD5Hash(m_filePath, m_logger, success);
            if (!success)
            {
				if (m_logger)m_logger->logError("Error calculating md5 hash of file: " + m_filePath);
				return;
			}
            if (md5 != m_md5Hash && m_md5Hash!="")
            {
				m_fileChanged.store(true);
                if(m_logger)m_logger->logInfo("File change detected");
			}
            m_md5Hash = md5;
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
        bool ManagedFileChangeWatcher::setup(const std::string& targetFile, Log::LogObject* parentLogger)
        {
            delete m_logger;
            if (parentLogger)
            {
                m_logger = new Log::LogObject(*parentLogger, "Filewatcher: "+targetFile);
                m_logger->logInfo("Setting up file change watcher");
            }

            return restart(targetFile);
        }
        bool ManagedFileChangeWatcher::restart(const std::string& targetFile)
        {
            if (m_databaseFileWatcher)
            {
                m_databaseFileWatcher->stopWatching();
                delete m_databaseFileWatcher;
                m_databaseFileWatcher = nullptr;
            }
            m_databaseFileWatcher = new FileChangeWatcher(targetFile);
            bool success = m_databaseFileWatcher->setup(m_logger);
            DWORD lastError = m_databaseFileWatcher->getSetupError();
            JD_UNUSED(lastError);
            if (!success)
            {
                if(m_logger)m_logger->logError("Initializing file change monitoring. GetLastError() =  " + std::to_string(lastError) + " : " + Utilities::getLastErrorString(lastError));
				return false;
			}
            m_databaseFileWatcher->startWatching();
            return success;
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