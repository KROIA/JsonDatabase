#include "manager/JDManagerFileSystem.h"
#include "manager/JDManager.h"
#include "utilities/filesystem/StringZipper.h"
#include "utilities/SystemCommand.h"
#include "utilities/JDUniqueMutexLock.h"

#include <QtZlib/zlib.h>
#if JD_ACTIVE_JSON == JD_JSON_QT
#include <QJsonDocument>
#include <QJsonArray>
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
#include "Json/JsonValue.h"
#include "Json/JsonDeserializer.h"
#include "Json/JsonSerializer.h"
#endif
#include <QFile>
#include <QDir>
#include <QtEndian>
#include <string>

namespace JsonDatabase
{
    namespace Internal
    {
        const std::string JDManagerFileSystem::s_jsonFileEnding = ".json";

        JDManagerFileSystem::JDManagerFileSystem(
            JDManager& manager,
            std::mutex& mtx)
			: m_logger(nullptr)
            , m_databaseFileName("data")
            , m_manager(manager)
            , m_mutex(mtx)
            , m_fileLock(nullptr)
            , m_slowUpdateCounter(-1) // -1 to trigger first update
            , m_middleUpdateCounter(-1) // -1 to trigger first update
            , m_userRegistration()
           // , m_databaseLoginFileLock(nullptr)
		{

        }
        JDManagerFileSystem::~JDManagerFileSystem()
        {
            stop();            
            delete m_logger;
        }

        void JDManagerFileSystem::setParentLogger(Log::LogObject* parentLogger)
        {
            if (parentLogger)
            {
                if (m_logger)
                    delete m_logger;
                m_logger = new Log::LogObject(*parentLogger,"Filesystem manager");
                m_userRegistration.setParentLogger(m_logger, "User registration");
            }
        }

        bool JDManagerFileSystem::setup(const std::string& databasePath,
                                        const std::string& databaseName)
        {
            bool success = true;
            m_databasePath = databasePath;
            m_databaseName = databaseName;

            success &= makeDatabaseDirs();
            success &= makeDatabaseFiles();
            
            restartDatabaseFileWatcher();
            m_userRegistration.setDatabasePath(m_manager.getDatabasePath());
            m_userRegistration.createFiles();

            logOnDatabase();
            return success;
        }
        bool JDManagerFileSystem::stop()
        {
            m_fileWatcher.stop();
            if (m_fileLock)
                delete m_fileLock;
            m_fileLock = nullptr;
            logOffDatabase();
            return true;
        }


        void JDManagerFileSystem::setDatabaseName(const std::string& name)
        {
            JDM_UNIQUE_LOCK_P;
            m_databaseName = name;
            restartDatabaseFileWatcher();
        }

        void JDManagerFileSystem::setDatabasePath(const std::string& path)
        {
            JDM_UNIQUE_LOCK_P;
            if (path == m_databasePath)
                return;
            m_manager.onDatabasePathChange(m_databasePath, path);
            logOffDatabase();
            m_databasePath = path;
            makeDatabaseDirs();
            makeDatabaseFiles();
            m_userRegistration.setDatabasePath(m_manager.getDatabasePath());
            logOnDatabase();
            restartDatabaseFileWatcher();
        }
        const std::string& JDManagerFileSystem::getDatabaseName() const
        {
            return m_databaseName;
        }
        const std::string& JDManagerFileSystem::getDatabaseFileName() const
        {
            return m_databaseFileName;
        }
        std::string JDManagerFileSystem::getDatabasePath() const
        {
            return m_databasePath + "\\" + m_databaseName;
        }
        std::string JDManagerFileSystem::getDatabaseFilePath() const
        {
            return  getDatabasePath() + "\\" + m_databaseFileName + Internal::JDManagerFileSystem::getJsonFileEnding();
        }
        bool JDManagerFileSystem::isLoggedOnDatabase() const
        {
            return m_userRegistration.isUserRegistered();
        }


        const std::string& JDManagerFileSystem::getJsonFileEnding()
        {
            return s_jsonFileEnding;
        }
     
        void JDManagerFileSystem::logOnDatabase()
        {
            Utilities::JDUser &user = m_manager.m_user;
            std::string sessionID;
            m_userRegistration.registerUser(user, sessionID);
            user.setSessionID(sessionID);            
        }
        void JDManagerFileSystem::logOffDatabase()
        {
            m_userRegistration.unregisterUser();
        }




        bool JDManagerFileSystem::makeDatabaseDirs() const
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            bool success = true;

            std::string path = getDatabasePath();
            QDir dir(path.c_str());
            if (!dir.exists())
            {
                if(m_logger) m_logger->logInfo("Creating database folder: " + path);
                QDir d;
                d.mkpath(path.c_str());
            }


            bool exists = dir.exists();
            if (!exists)
            {
                if (m_logger)m_logger->logError("bool JDManagerFileSystem::makeDatabaseDirs() Can't create database folder: " + path);
            }
            success &= exists;

            return success;
        }
        bool JDManagerFileSystem::makeDatabaseFiles() const
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            QFile file(m_manager.getDatabaseFilePath().c_str());
            if (!file.exists())
            {
                if(m_logger) m_logger->logInfo("Creating database file: " + m_manager.getDatabaseFilePath());
                // Create empty data
                LockedFileAccessor fileAccessor(getDatabasePath(), getDatabaseFileName(), getJsonFileEnding(), m_logger);
                fileAccessor.setProgress(nullptr);
                Error fileError = fileAccessor.lock(LockedFileAccessor::AccessMode::write);

                if (fileError != Error::none)
                {
                    if(m_logger) m_logger->logError(std::string("bool JDManager::saveObjects_internal(const std::vector<JDObject>& objList, unsigned int timeoutMillis): Error: ") + errorToString(fileError));
                    return false;
                }


                JsonArray jsonData{};
                fileError = fileAccessor.writeJsonFile(jsonData);
                if (fileError != Error::none)
                {
                    if(m_logger) m_logger->logError(std::string("bool JDManager::saveObjects_internal(const std::vector<JDObject>& objList, unsigned int timeoutMillis): Error: ") + errorToString(fileError));
                    return false;
                }
                else
                    return true;
            }
            return true;
        }

        bool JDManagerFileSystem::deleteDir(const std::string& dir) const
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            int ret = SystemCommand::execute("rd /s /q \"" + dir + "\"");
            JD_UNUSED(ret);
            QDir folder(dir.c_str());
            if (folder.exists())
            {
                if(m_logger) m_logger->logError("bool JDManagerFileSystem::deleteDir("+dir+") Folder could not be deleted : " + dir);
                return false;
            }
            return true;
        }
        bool JDManagerFileSystem::deleteFile(const std::string& file) const
        {
            QFile f(file.c_str());
            if (f.exists())
                return f.remove();
            return true;
        }

        int JDManagerFileSystem::tryToClearUnusedFileLocks() const
        {
            return m_userRegistration.unregisterInactiveUsers();
        }

        ManagedFileChangeWatcher& JDManagerFileSystem::getDatabaseFileWatcher()
        {
            return m_fileWatcher;
        }
        void JDManagerFileSystem::restartDatabaseFileWatcher()
        {
            m_fileWatcher.setup(getDatabaseFilePath(),m_logger);
        }
        
        void JDManagerFileSystem::update()
        {
            if(m_slowUpdateCounter >= 100)
			{
				m_slowUpdateCounter = 0;
				tryToClearUnusedFileLocks();
			}
            if (m_middleUpdateCounter >= 10)
            {
				m_middleUpdateCounter = 0;
                std::vector<Utilities::JDUser> loggedOnUsers;
                std::vector<Utilities::JDUser> loggedOffUsers;
                m_userRegistration.checkForUserChange(loggedOnUsers, loggedOffUsers);
            }

            ++m_slowUpdateCounter;
            ++m_middleUpdateCounter;

            if (m_fileWatcher.hasFileChanged())
            {
                //m_manager.getSignals().databaseFileChanged.emitSignal();
                emit m_manager.databaseFileChanged();
                m_fileWatcher.clearHasFileChanged();
            }
        }
    }
}