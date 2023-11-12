#include "manager/JDManagerFileSystem.h"
#include "manager/JDManager.h"
#include "utilities/filesystem/StringZipper.h"
#include "utilities/SystemCommand.h"
#include "utilities/filesystem/FileChangeWatcher.h"
#include "utilities/JDUniqueMutexLock.h"

#include <QtZlib/zlib.h>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QDir>

namespace JsonDatabase
{
    namespace Internal
    {
        const std::string JDManagerFileSystem::s_jsonFileEnding = ".json";

        JDManagerFileSystem::JDManagerFileSystem(
            JDManager& manager,
            std::mutex& mtx)
			: m_manager(manager)
            , m_mutex(mtx)
            , m_fileLock(nullptr)
            , m_databaseFileWatcher(nullptr)
		{
            
        }
        JDManagerFileSystem::~JDManagerFileSystem()
        {
            if(m_fileLock)
                delete m_fileLock;
            if (m_databaseFileWatcher)
            {
                m_databaseFileWatcher->stopWatching();
                delete m_databaseFileWatcher;
            }
        }

        void JDManagerFileSystem::setup()
        {
            makeDatabaseDirs();
            makeDatabaseFiles();
            restartFileWatcher();
        }
     


        bool JDManagerFileSystem::lockFile(
            const std::string& directory,
            const std::string& fileName,
            FileReadWriteLock::Access direction,
            bool& wasLockedForWritingByOther) const
        {
            if (m_fileLock)
            {
                JD_CONSOLE_FUNCTION("Lock already aquired\n");
                return false;
            }

            m_fileLock = new FileReadWriteLock(directory, fileName);

            if (!m_fileLock->lock(direction))
            {
                JD_CONSOLE_FUNCTION("Can't aquire lock for: " << directory << "\\" << fileName << "\n");
                wasLockedForWritingByOther = m_fileLock->wasLockedForWritingByOther();
                delete m_fileLock;
                m_fileLock = nullptr;
                return false;
            }
            wasLockedForWritingByOther = m_fileLock->wasLockedForWritingByOther();
            if (m_fileLock->getLastError() != FileLock::Error::none)
            {
                JD_CONSOLE_FUNCTION("Lock error: " << m_fileLock->getLastErrorStr() + "\n");
            }
            return true;
        }
        bool JDManagerFileSystem::lockFile(
            const std::string& directory,
            const std::string& fileName,
            FileReadWriteLock::Access direction,
            bool& wasLockedForWritingByOther,
            unsigned int timeoutMillis) const
        {
            if (m_fileLock)
            {
                JD_CONSOLE_FUNCTION("Lock already aquired\n");
                return false;
            }

            m_fileLock = new FileReadWriteLock(directory, fileName);

            if (!m_fileLock->lock(direction, timeoutMillis))
            {
                JD_CONSOLE_FUNCTION("Timeout while trying to aquire file lock for: " << directory << "\\" << fileName << "\n");
                wasLockedForWritingByOther = m_fileLock->wasLockedForWritingByOther();
                delete m_fileLock;
                m_fileLock = nullptr;
                return false;
            }
            wasLockedForWritingByOther = m_fileLock->wasLockedForWritingByOther();
            if (m_fileLock->getLastError() != FileLock::Error::none)
            {
                JD_CONSOLE_FUNCTION("Lock error: " << m_fileLock->getLastErrorStr() + "\n");
            }
            return true;
        }
        bool JDManagerFileSystem::unlockFile() const
        {
            //JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2); 
            //JD_GENERAL_PROFILING_FUNCTION_C(profiler::colors::Red);

            if (!m_fileLock)
                return true;

            m_fileLock->unlock();
            delete m_fileLock;
            m_fileLock = nullptr;

            return true;
        }
        bool JDManagerFileSystem::isFileLockedByOther(
            const std::string& directory,
            const std::string& fileName,
            FileReadWriteLock::Access accessType) const
        {
            FileReadWriteLock::Access a = FileReadWriteLock::Access::unknown;
            if (!m_fileLock)
            {
                FileReadWriteLock lock(directory, fileName);
                a = lock.getAccessStatus();
            }
            else
            {
                m_fileLock->getAccessStatus();
            }
            return a == accessType;
        }
        FileLock::Error JDManagerFileSystem::getLastLockError() const
        {
            if (!m_fileLock)
                return FileLock::Error::none;
            return m_fileLock->getLastError();
        }
        const std::string& JDManagerFileSystem::getLastLockErrorStr() const
        {
            if (!m_fileLock)
            {
                static const std::string dummy;
                return dummy;
            }
            return m_fileLock->getLastErrorStr();
        }

        bool JDManagerFileSystem::writeJsonFile(
            const std::vector<QJsonObject>& jsons,
            const std::string& directory,
            const std::string& fileName,
            const std::string& fileEnding,
            bool zipFormat,
            bool lockedRead) const
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_5);
            JD_GENERAL_PROFILING_NONSCOPED_BLOCK("std::vector to QJsonArray", JD_COLOR_STAGE_6);
            QJsonArray jsonArray;

            // Convert QJsonObject instances to QJsonValue and add them to QJsonArray
            for (const auto& jsonObject : jsons) {
                jsonArray.append(QJsonValue(jsonObject));
            }
            JD_GENERAL_PROFILING_END_BLOCK;


            JD_GENERAL_PROFILING_NONSCOPED_BLOCK("toJson", JD_COLOR_STAGE_6);
            QJsonDocument jsonDocument(jsonArray);
            // Convert QJsonDocument to a QByteArray for writing to a file
            QByteArray data = jsonDocument.toJson(QJsonDocument::JsonFormat::Indented);
            JD_GENERAL_PROFILING_END_BLOCK;


            // Write the JSON data to the file
            if (zipFormat)
            {
                JD_GENERAL_PROFILING_NONSCOPED_BLOCK("compressing data", JD_COLOR_STAGE_6);
                QByteArray fileData;
                StringZipper::compressString(data, fileData);
                JD_GENERAL_PROFILING_END_BLOCK;

                return writeFile(fileData, directory, fileName, s_jsonFileEnding, lockedRead);
            }
            else
            {
                return writeFile(data, directory, fileName, s_jsonFileEnding, lockedRead);
            }
            return false;
        }

        bool JDManagerFileSystem::writeJsonFile(
            const QJsonObject& json,
            const std::string& directory,
            const std::string& fileName,
            const std::string& fileEnding,
            bool zipFormat,
            bool lockedRead) const
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_5);

            JD_GENERAL_PROFILING_NONSCOPED_BLOCK("toJson", JD_COLOR_STAGE_6);
            QJsonDocument jsonDocument(json);
            // Convert QJsonDocument to a QByteArray for writing to a file
            QByteArray data = jsonDocument.toJson(QJsonDocument::JsonFormat::Indented);
            JD_GENERAL_PROFILING_END_BLOCK;


            // Write the JSON data to the file
            if (zipFormat)
            {
                JD_GENERAL_PROFILING_NONSCOPED_BLOCK("compressing data", JD_COLOR_STAGE_6);
                QByteArray fileData;
                StringZipper::compressString(data, fileData);
                JD_GENERAL_PROFILING_END_BLOCK;

                return writeFile(fileData, directory, fileName, s_jsonFileEnding, lockedRead);
            }
            else
            {
                return writeFile(data, directory, fileName, s_jsonFileEnding, lockedRead);
            }
            return false;
        }

        bool JDManagerFileSystem::readJsonFile(
            std::vector<QJsonObject>& jsonsOut,
            const std::string& directory,
            const std::string& fileName,
            const std::string& fileEnding,
            bool zipFormat,
            bool lockedRead) const
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_5);
            QByteArray fileData;
            if (!readFile(fileData, directory, fileName, fileEnding, lockedRead))
            {
                return false;
            }

            // Parse the JSON data
            QJsonDocument jsonDocument;
            if (zipFormat)
            {
                JD_GENERAL_PROFILING_NONSCOPED_BLOCK("uncompressing data", JD_COLOR_STAGE_6);
                QString uncompressed;
                if (StringZipper::decompressString(fileData, uncompressed))
                {
                    JD_GENERAL_PROFILING_END_BLOCK;
                    JD_GENERAL_PROFILING_NONSCOPED_BLOCK("import json", JD_COLOR_STAGE_6);
                    jsonDocument = QJsonDocument::fromJson(uncompressed.toUtf8());
                    JD_GENERAL_PROFILING_END_BLOCK;
                }
                else
                {
                    JD_GENERAL_PROFILING_END_BLOCK;
                    JD_GENERAL_PROFILING_NONSCOPED_BLOCK("import json", JD_COLOR_STAGE_6);
                    jsonDocument = QJsonDocument::fromJson(fileData);
                    JD_GENERAL_PROFILING_END_BLOCK;
                }
            }
            else
            {
                JD_GENERAL_PROFILING_NONSCOPED_BLOCK("import json", JD_COLOR_STAGE_6);
                jsonDocument = QJsonDocument::fromJson(fileData);
                JD_GENERAL_PROFILING_END_BLOCK;
            }



            // Check if the JSON document is an array
            if (jsonDocument.isArray()) {
                JD_GENERAL_PROFILING_NONSCOPED_BLOCK("QJsonArray to std::vector", JD_COLOR_STAGE_6);
                QJsonArray jsonArray = jsonDocument.array();

                // Iterate through the array and add QJsonObjects to the vector
                jsonsOut.reserve(jsonArray.size());
                for (const auto& jsonValue : jsonArray) {
                    if (jsonValue.isObject()) {
                        jsonsOut.push_back(jsonValue.toObject());
                    }
                }
                JD_GENERAL_PROFILING_END_BLOCK;
                return true;
            }
            else {
                JD_CONSOLE_FUNCTION("Error: JSON document from file: " << inputFile << " is not an array\n");
                return false;
            }

            return false;
        }
        bool JDManagerFileSystem::readJsonFile(
            QJsonObject& objOut,
            const std::string& directory,
            const std::string& fileName,
            const std::string& fileEnding,
            bool zipFormat,
            bool lockedRead) const
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_5);
            QByteArray fileData;
            if (!readFile(fileData, directory, fileName, fileEnding, lockedRead))
            {
                return false;
            }
            QJsonParseError jsonError;

            QJsonDocument document;
            if (zipFormat)
            {
                JD_GENERAL_PROFILING_NONSCOPED_BLOCK("uncompressing data", JD_COLOR_STAGE_6);
                QString uncompressed;
                if (StringZipper::decompressString(fileData, uncompressed))
                {
                    JD_GENERAL_PROFILING_END_BLOCK;
                    JD_GENERAL_PROFILING_NONSCOPED_BLOCK("import json", JD_COLOR_STAGE_6);
                    document = QJsonDocument::fromJson(uncompressed.toUtf8());
                    JD_GENERAL_PROFILING_END_BLOCK;
                }
                else
                {
                    JD_GENERAL_PROFILING_END_BLOCK;
                    JD_GENERAL_PROFILING_NONSCOPED_BLOCK("import json", JD_COLOR_STAGE_6);
                    document = QJsonDocument::fromJson(fileData);
                    JD_GENERAL_PROFILING_END_BLOCK;
                }
            }
            else
            {
                JD_GENERAL_PROFILING_NONSCOPED_BLOCK("import json", JD_COLOR_STAGE_6);
                document = QJsonDocument::fromJson(fileData);
                JD_GENERAL_PROFILING_END_BLOCK;
            }
            if (jsonError.error != QJsonParseError::NoError)
            {
                JD_CONSOLE_FUNCTION("Can't read Jsonfile: " << jsonError.errorString().toStdString().c_str() << "\n");
                return false;
            }
            if (document.isObject())
            {
                objOut = document.object();
                return true;
            }

            return false;

            /*QFile file((m_databasePath + "\\" + relativePath + fileEnding).c_str());
            if( file.open( QIODevice::ReadOnly ) )
            {
                QByteArray bytes = file.readAll();
                file.close();

                QJsonParseError jsonError;
                QJsonDocument document = QJsonDocument::fromJson( bytes, &jsonError );
                if( jsonError.error != QJsonParseError::NoError )
                {
                    JD_CONSOLE_FUNCTION("Can't read Jsonfile: " << jsonError.errorString().toStdString().c_str() << "\n");
                    return false;
                }
                if( document.isObject() )
                {
                    obj = document.object();
                    return true;
                }
            }
            JD_CONSOLE_FUNCTION("Can't open file: "<<(m_databasePath+"\\"+relativePath+fileEnding).c_str()<<"\n");
            return false;*/
        }

        bool JDManagerFileSystem::readFile(
            QByteArray& fileDataOut,
            const std::string& directory,
            const std::string& fileName,
            const std::string& fileEnding,
            bool lockedRead) const
        {
            JDFILE_IO_PROFILING_FUNCTION(JD_COLOR_STAGE_5);
            if (lockedRead)
            {
                bool lockedByOther = false;
                if (!lockFile(directory, fileName, FileReadWriteLock::Access::read, lockedByOther))
                {
                    JD_CONSOLE_FUNCTION(" Can't lock file\n");
                    return false;
                }
            }
            /*
            JDFILE_IO_PROFILING_NONSCOPED_BLOCK("open file", JD_COLOR_STAGE_6);
            std::string filePath = directory + "\\" + fileName + fileEnding;
            QFile file(filePath.c_str());
            if (file.open(QIODevice::ReadOnly))
            {
                JDFILE_IO_PROFILING_END_BLOCK;
                JDFILE_IO_PROFILING_NONSCOPED_BLOCK("read from file", JD_COLOR_STAGE_6);
                fileDataOut = file.readAll();
                file.close();
                JDFILE_IO_PROFILING_END_BLOCK;
                if (lockedRead)
                    unlockFile();
                return true;
            }
            else
            {
                JDFILE_IO_PROFILING_END_BLOCK;
            }
            JD_CONSOLE_FUNCTION("Can't open file: " << filePath.c_str() << "\n");
            if (lockedRead)
                unlockFile();
            return false;*/


            JDFILE_IO_PROFILING_NONSCOPED_BLOCK("open file", JD_COLOR_STAGE_6);
            std::string filePath = directory + "\\" + fileName + fileEnding;
            HANDLE fileHandle = CreateFile(
                filePath.c_str(),
                GENERIC_READ,
                FILE_SHARE_READ,
                nullptr,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                nullptr
            );

            if (fileHandle == INVALID_HANDLE_VALUE) {
                JDFILE_IO_PROFILING_END_BLOCK;
                JD_CONSOLE_FUNCTION("Can't open file: " << filePath.c_str() << "\n");
                if (lockedRead)
                    unlockFile();
                return false;
            }
            JDFILE_IO_PROFILING_END_BLOCK;
            JDFILE_IO_PROFILING_NONSCOPED_BLOCK("read from file", JD_COLOR_STAGE_6);
            DWORD fileSize = GetFileSize(fileHandle, nullptr);
            if (fileSize == INVALID_FILE_SIZE) {
                CloseHandle(fileHandle);
                JD_CONSOLE_FUNCTION("Can't get filesize of: " << filePath.c_str() << "\n");
                JDFILE_IO_PROFILING_END_BLOCK;
                if (lockedRead)
                    unlockFile();
                return false;
            }

            std::string content(fileSize, '\0');
            DWORD bytesRead;
            BOOL readResult = ReadFile(
                fileHandle,
                &content[0],
                fileSize,
                &bytesRead,
                nullptr
            );

            CloseHandle(fileHandle);
            JDFILE_IO_PROFILING_END_BLOCK;
            if (lockedRead)
                unlockFile();

            if (!readResult) {
                JD_CONSOLE_FUNCTION("Can't read file: " << filePath.c_str() << "\n");
            }

            fileDataOut = content.c_str();
            return true;
        }
        bool JDManagerFileSystem::writeFile(
            const QByteArray& fileData,
            const std::string& directory,
            const std::string& fileName,
            const std::string& fileEnding,
            bool lockedRead) const
        {
            JDFILE_IO_PROFILING_FUNCTION(JD_COLOR_STAGE_5);
            if (lockedRead)
            {
                bool lockedByOther = false;
                if (!lockFile(directory, fileName, FileReadWriteLock::Access::write, lockedByOther))
                {
                    JD_CONSOLE_FUNCTION(" Can't lock file\n");
                    return false;
                }
            }

            /*
            // Open the file for writing
            JDFILE_IO_PROFILING_NONSCOPED_BLOCK("open file", JD_COLOR_STAGE_6);
            std::string filePath = directory + "\\" + fileName + fileEnding;
            QFile file(filePath.c_str());
            if (file.open(QIODevice::WriteOnly)) {
                // Write the JSON data to the file
                JDFILE_IO_PROFILING_END_BLOCK;
                JDFILE_IO_PROFILING_NONSCOPED_BLOCK("write to file", JD_COLOR_STAGE_6);
                file.write(fileData);
                file.close();
                JDFILE_IO_PROFILING_END_BLOCK;
                if (lockedRead)
                    unlockFile();
                return true;
            }
            else {
                JDFILE_IO_PROFILING_END_BLOCK;
                JD_CONSOLE_FUNCTION("Error: Could not open file " << filePath << " for writing\n");
            }
            if (lockedRead)
                unlockFile();
            return false;*/

            // Open the file for writing
            JDFILE_IO_PROFILING_NONSCOPED_BLOCK("open file", JD_COLOR_STAGE_6);
            std::string filePath = directory + "\\" + fileName + fileEnding;
            HANDLE fileHandle = CreateFile(
                filePath.c_str(),
                GENERIC_WRITE,
                0,
                nullptr,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                nullptr
            );

            if (fileHandle == INVALID_HANDLE_VALUE) {
                JDFILE_IO_PROFILING_END_BLOCK;
                JD_CONSOLE_FUNCTION("Error: Could not open file " << filePath << " for writing\n");
                // Error opening file
                if (lockedRead)
                    unlockFile();
                return false;
            }
            JDFILE_IO_PROFILING_END_BLOCK;

            JDFILE_IO_PROFILING_NONSCOPED_BLOCK("write to file", JD_COLOR_STAGE_6);
            // Write the content to the file
            DWORD bytesWritten;
            BOOL writeResult = WriteFile(
                fileHandle,
                fileData.constData(),
                fileData.size(),
                &bytesWritten,
                nullptr
            );


            // Close the file handle
            CloseHandle(fileHandle);
            JDFILE_IO_PROFILING_END_BLOCK;
            if (lockedRead)
                unlockFile();

            if (!writeResult) {
                // Error writing to file
                JD_CONSOLE_FUNCTION("Error: Could not write to file " << filePath << "\n");
                return false;
            }

            return true;
        }


        bool JDManagerFileSystem::makeDatabaseDirs() const
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
                bool success = true;

            std::string path = m_manager.getDatabasePath();
            QDir dir(path.c_str());
            if (!dir.exists())
            {
                QDir d;
                d.mkpath(path.c_str());
            }


            bool exists = dir.exists();
            if (!exists)
            {
                JD_CONSOLE_FUNCTION("Can't create database folder: " << path.c_str() << "\n");
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
                if (file.open(QIODevice::WriteOnly))
                {
					file.close();
					return true;
				}
                else
                {
					JD_CONSOLE_FUNCTION("Can't create database file: " << m_manager.getDatabaseFilePath().c_str() << "\n");
				}
            }
            return false;
        }

        bool JDManagerFileSystem::deleteDir(const std::string& dir) const
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            int ret = SystemCommand::execute("rd /s /q \"" + dir + "\"");
            QDir folder(dir.c_str());
            if (folder.exists())
            {
                JD_CONSOLE_FUNCTION(" Folder could not be deleted: " + dir + "\n");
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


        void JDManagerFileSystem::restartFileWatcher()
        {
            if (m_databaseFileWatcher)
            {
                m_databaseFileWatcher->stopWatching();
                delete m_databaseFileWatcher;
                m_databaseFileWatcher = nullptr;
            }
            m_databaseFileWatcher = new FileChangeWatcher(m_manager.getDatabaseFilePath());
            m_databaseFileWatcher->startWatching();
        }

        bool JDManagerFileSystem::isFileWatcherRunning() const
        {
            if (!m_databaseFileWatcher)
                return false;
            return m_databaseFileWatcher->isWatching();
        }
        void JDManagerFileSystem::stopFileWatcher()
        {
            if (!m_databaseFileWatcher)
                return;
            m_databaseFileWatcher->stopWatching();
        }
        bool JDManagerFileSystem::fileWatcherHasFileChanged() const
        {
            if (!m_databaseFileWatcher)
                return false;
            return m_databaseFileWatcher->hasFileChanged();
        }
        void JDManagerFileSystem::clearFileWatcherHasFileChanged()
        {
            if (!m_databaseFileWatcher)
                return;
            m_databaseFileWatcher->clearFileChangedFlag();
        }

        void JDManagerFileSystem::pauseFileWatcher()
        {
            if (!m_databaseFileWatcher)
                return;
            m_databaseFileWatcher->pause();
        }
        void JDManagerFileSystem::unpauseFileWatcher()
        {
            if (!m_databaseFileWatcher)
                return;
            m_databaseFileWatcher->unpause();
        }
        bool JDManagerFileSystem::isFileWatcherPaused() const
        {
            if (!m_databaseFileWatcher)
				return false;
			return m_databaseFileWatcher->isPaused();
        }
        
    }
}