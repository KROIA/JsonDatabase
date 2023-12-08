#include "manager/JDManagerFileSystem.h"
#include "manager/JDManager.h"
#include "utilities/filesystem/StringZipper.h"
#include "utilities/SystemCommand.h"
#include "utilities/JDUniqueMutexLock.h"

#include <QtZlib/zlib.h>
#if JD_ACTIVE_JSON == JD_JSON_QT
#include <QJsonDocument>
#include <QJsonArray>
#elif JD_ACTIVE_JSON == JD_JSON_GLAZE || JD_ACTIVE_JSON == JD_JSON_INTERNAL
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
            const std::string& databasePath,
            const std::string& databaseName,
            JDManager& manager,
            std::mutex& mtx)
			: m_databasePath(databasePath)
            , m_databaseName(databaseName)
            , m_databaseFileName("data")
            , m_manager(manager)
            , m_mutex(mtx)
            , m_fileLock(nullptr)
            , m_slowUpdateCounter(-1) // -1 to trigger first update
            , m_userRegistration()
           // , m_databaseLoginFileLock(nullptr)
		{
            
        }
        JDManagerFileSystem::~JDManagerFileSystem()
        {
            m_fileWatcher.stop();
            if(m_fileLock)
                delete m_fileLock;

            logOffDatabase();
        }


        bool JDManagerFileSystem::setup()
        {
            bool success = true;
            success &= makeDatabaseDirs();
            success &= makeDatabaseFiles();
            
            restartDatabaseFileWatcher();
            m_userRegistration.setDatabasePath(m_manager.getDatabasePath());
            m_userRegistration.createFiles();

            logOnDatabase();
            return success;
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
            m_userRegistration.createFiles();
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
                QDir d;
                d.mkpath(path.c_str());
            }


            bool exists = dir.exists();
            if (!exists)
            {
                JD_CONSOLE("bool JDManagerFileSystem::makeDatabaseDirs() Can't create database folder: " << path.c_str() << "\n");
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
                // Create empty data
                LockedFileAccessor fileAccessor(getDatabasePath(), getDatabaseFileName(), getJsonFileEnding());
                fileAccessor.setProgress(nullptr);
                LockedFileAccessor::Error fileError = fileAccessor.lock(LockedFileAccessor::AccessMode::write);

                if (fileError != LockedFileAccessor::Error::none)
                {
                    JD_CONSOLE("bool JDManager::saveObjects_internal(const std::vector<JDObject>& objList, unsigned int timeoutMillis): Error: " + LockedFileAccessor::getErrorStr(fileError) + "\n");
                    return false;
                }

#if JD_ACTIVE_JSON == JD_JSON_QT
                std::vector<QJsonObject> jsonData;
#elif JD_ACTIVE_JSON == JD_JSON_GLAZE || JD_ACTIVE_JSON == JD_JSON_INTERNAL
                JsonArray jsonData{};
#endif
                fileError = fileAccessor.writeJsonFile(jsonData);
                if (fileError != LockedFileAccessor::Error::none)
                {
                    JD_CONSOLE("bool JDManager::saveObject_internal(const std::vector<JDObject>& objList, unsigned int timeoutMillis): Error: " + LockedFileAccessor::getErrorStr(fileError) + "\n");
                    return false;
                }
                else
                    return true;
                /*
                if (file.open(QIODevice::WriteOnly))
                {
					file.close();
					return true;
				}
                else
                {
					JD_CONSOLE("bool JDManagerFileSystem::makeDatabaseFiles() Can't create database file: " << m_manager.getDatabaseFilePath().c_str() << "\n");
                    return false;
                }*/
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
                JD_CONSOLE("bool JDManagerFileSystem::deleteDir("<<dir<<") Folder could not be deleted : " + dir + "\n");
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
           /* const std::string lockFileEnding = FileLock::s_lockFileEnding;

            // Get all files in the database directory
            QDir dir(m_manager.getDatabasePath().c_str());
			dir.setFilter(QDir::Files | QDir::NoSymLinks);
			dir.setSorting(QDir::Name);
			QFileInfoList list = dir.entryInfoList();

			// Iterate through all files
			int count = 0;
			for (int i = 0; i < list.size(); ++i) {
				QFileInfo fileInfo = list.at(i);
				QString fileName = fileInfo.fileName();
				if (fileName.endsWith(lockFileEnding.c_str()))
				{
					// Check if the file is a lock file
					std::string filePath = fileInfo.absoluteFilePath().toStdString();
					if (!FileLock::isFileLocked(filePath))
					{
						// Delete the file
						QFile file(filePath.c_str());
						if (file.remove())
						{
                            QFile file2(filePath.c_str());
                            if (file2.exists())
							{
                                JD_CONSOLE_FUNCTION("Can't delete file: " << filePath.c_str() << "\n");
							}
                            else
                            {
                                JD_CONSOLE_FUNCTION("Deleted unused lock: " << filePath.c_str() << "\n");
                                ++count;
                            }
						}
					}
				}
			}
			return count;*/
        }

        ManagedFileChangeWatcher& JDManagerFileSystem::getDatabaseFileWatcher()
        {
            return m_fileWatcher;
        }
        void JDManagerFileSystem::restartDatabaseFileWatcher()
        {
            m_fileWatcher.setup(getDatabaseFilePath());
        }
        
        void JDManagerFileSystem::update()
        {
            if(m_slowUpdateCounter >= 100)
			{
				m_slowUpdateCounter = 0;
				tryToClearUnusedFileLocks();
			}
            ++m_slowUpdateCounter;
            if (m_fileWatcher.hasFileChanged())
            {
                m_manager.getSignals().databaseFileChanged.emitSignal();
                m_fileWatcher.clearHasFileChanged();
            }
        }
    }
}