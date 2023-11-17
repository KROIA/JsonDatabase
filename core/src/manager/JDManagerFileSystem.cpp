#include "manager/JDManagerFileSystem.h"
#include "manager/JDManager.h"
#include "utilities/filesystem/StringZipper.h"
#include "utilities/SystemCommand.h"
#include "utilities/JDUniqueMutexLock.h"

#include <QtZlib/zlib.h>
#ifdef JD_USE_QJSON
#include <QJsonDocument>
#include <QJsonArray>
#else
#include "Json/JsonValue.h"
#include "Json/JsonDeserializer.h"
#include "Json/JsonSerializer.h"
#endif
#include <QFile>
#include <QDir>
#include <QtEndian>

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
		{
            
        }
        JDManagerFileSystem::~JDManagerFileSystem()
        {
            m_fileWatcher.stop();
            if(m_fileLock)
                delete m_fileLock;
        }


        bool JDManagerFileSystem::setup()
        {
            bool success = true;
            success &= makeDatabaseDirs();
            success &= makeDatabaseFiles();
            restartDatabaseFileWatcher();
            return success;
        }

        const std::string& JDManagerFileSystem::getJsonFileEnding()
        {
            return s_jsonFileEnding;
        }
     


        bool JDManagerFileSystem::lockFile(
            const std::string& directory,
            const std::string& fileName,
            FileReadWriteLock::Access direction,
            bool& wasLockedForWritingByOther) const
        {
            if (m_fileLock)
            {
                JD_CONSOLE("bool JDManagerFileSystem::lockFile("
                    << directory<<", "
                    << fileName<<", "
                    << FileReadWriteLock::accessTypeToString(direction)
                    <<", bool) Lock already aquired\n");
                return false;
            }

            m_fileLock = new FileReadWriteLock(directory, fileName);

            FileLock::Error lockErr;
            if (!m_fileLock->lock(direction, wasLockedForWritingByOther, lockErr))
            {
                JD_CONSOLE("bool JDManagerFileSystem::lockFile("
                    << directory << ", "
                    << fileName << ", "
                    << FileReadWriteLock::accessTypeToString(direction)
                    << ", bool) Can't aquire lock for: " << directory << "\\" << fileName << "\n");
                delete m_fileLock;
                m_fileLock = nullptr;
                return false;
            }
            if (lockErr != FileLock::Error::none)
            {
                JD_CONSOLE("bool JDManagerFileSystem::lockFile("
                    << directory << ", "
                    << fileName << ", "
                    << FileReadWriteLock::accessTypeToString(direction)
                    << ", bool) Lock error: " << FileLock::getErrorStr(lockErr) + "\n");
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
                JD_CONSOLE("bool JDManagerFileSystem::lockFile("
                    << directory << ", "
                    << fileName << ", "
                    << FileReadWriteLock::accessTypeToString(direction)
                    << ", bool, timeout="<< timeoutMillis<<"ms) Lock already aquired\n");
                return false;
            }

            m_fileLock = new FileReadWriteLock(directory, fileName);
            FileLock::Error lockErr;
            if (!m_fileLock->lock(direction, timeoutMillis, wasLockedForWritingByOther, lockErr))
            {
                JD_CONSOLE("bool JDManagerFileSystem::lockFile("
                    << directory << ", "
                    << fileName << ", "
                    << FileReadWriteLock::accessTypeToString(direction)
                    << ", bool, timeout=" << timeoutMillis << "ms) Timeout while trying to aquire file lock for: " << directory << "\\" << fileName << "\n");
                delete m_fileLock;
                m_fileLock = nullptr;
                return false;
            }
            if (lockErr != FileLock::Error::none)
            {
                JD_CONSOLE("bool JDManagerFileSystem::lockFile("
                    << directory << ", "
                    << fileName << ", "
                    << FileReadWriteLock::accessTypeToString(direction)
                    << ", bool, timeout=" << timeoutMillis << "ms) Lock error: " << FileLock::getErrorStr(lockErr) + "\n");
            }
            return true;
        }
        bool JDManagerFileSystem::unlockFile() const
        {
            //JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2); 
            //JD_GENERAL_PROFILING_FUNCTION_C(profiler::colors::Red);

            if (!m_fileLock)
                return true;
            FileLock::Error lockErr;
            m_fileLock->unlock(lockErr);
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
#ifdef JD_USE_QJSON
        bool JDManagerFileSystem::writeJsonFile(
            const std::vector<QJsonObject>& jsons,
            const std::string& directory,
            const std::string& fileName,
            const std::string& fileEnding,
            bool zipFormat,
            bool lockedRead) const
#else
        bool JDManagerFileSystem::writeJsonFile(
            const JsonArray& jsons,
            const std::string& directory,
            const std::string& fileName,
            const std::string& fileEnding,
            bool zipFormat,
            bool lockedRead) const
#endif
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_5);
#ifdef JD_USE_QJSON
            JD_GENERAL_PROFILING_NONSCOPED_BLOCK("std::vector to QJsonArray", JD_COLOR_STAGE_6);
            QJsonArray jsonArray;

            // Convert QJsonObject instances to QJsonValue and add them to QJsonArray
            for (const auto& jsonObject : jsons) {
                jsonArray.append(QJsonValue(jsonObject));
            }
            JD_GENERAL_PROFILING_END_BLOCK;
#endif


            JD_GENERAL_PROFILING_NONSCOPED_BLOCK("toJson", JD_COLOR_STAGE_6);
            QByteArray data;
#ifdef JD_USE_QJSON
            QJsonDocument jsonDocument(jsonArray);
            // Convert QJsonDocument to a QByteArray for writing to a file
            data = jsonDocument.toJson(QJsonDocument::JsonFormat::Indented);
#else
            JsonSerializer serializer;
            data = QByteArray::fromStdString(serializer.serializeArray(jsons));
#endif
            JD_GENERAL_PROFILING_END_BLOCK;

            m_fileWatcher.pause();
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
            m_fileWatcher.unpause();
            return false;
        }

#ifdef JD_USE_QJSON
        bool JDManagerFileSystem::writeJsonFile(
            const QJsonObject& json,
            const std::string& directory,
            const std::string& fileName,
            const std::string& fileEnding,
            bool zipFormat,
            bool lockedRead) const
#else
        bool JDManagerFileSystem::writeJsonFile(
            const JsonValue& json,
            const std::string& directory,
            const std::string& fileName,
            const std::string& fileEnding,
            bool zipFormat,
            bool lockedRead) const
#endif
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_5);


            JD_GENERAL_PROFILING_NONSCOPED_BLOCK("toJson", JD_COLOR_STAGE_6);
            QByteArray data;
#ifdef JD_USE_QJSON
            QJsonDocument jsonDocument(json);
            // Convert QJsonDocument to a QByteArray for writing to a file
            data = jsonDocument.toJson(QJsonDocument::JsonFormat::Indented);
#else
            JsonSerializer serializer;
            data = QByteArray::fromStdString(serializer.serializeValue(json));
#endif
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




        
#ifdef JD_USE_QJSON
        bool JDManagerFileSystem::readJsonFile(
            std::vector<QJsonObject>& jsonsOut,
            const std::string& directory,
            const std::string& fileName,
            const std::string& fileEnding,
            bool zipFormat,
            bool lockedRead) const
#else
        bool JDManagerFileSystem::readJsonFile(
            JsonArray& jsonsOut,
            const std::string& directory,
            const std::string& fileName,
            const std::string& fileEnding,
            bool zipFormat,
            bool lockedRead) const
#endif
        {
            
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_5);
            QByteArray fileData;
            if (!readFile(fileData, directory, fileName, fileEnding, lockedRead))
            {
                return false;
            }
           
            // Parse the JSON data
#ifdef JD_USE_QJSON
            QJsonDocument jsonDocument;
#else
            JsonDeserializer deserializer;
            JsonValue deserialized;
#endif
            if (zipFormat)
            {
                JD_GENERAL_PROFILING_NONSCOPED_BLOCK("uncompressing data", JD_COLOR_STAGE_6);
                QString uncompressed;
                if (StringZipper::decompressString(fileData, uncompressed))
                {
                    JD_GENERAL_PROFILING_END_BLOCK;
                    JD_GENERAL_PROFILING_NONSCOPED_BLOCK("import json", JD_COLOR_STAGE_6);
#ifdef JD_USE_QJSON
                    jsonDocument = QJsonDocument::fromJson(uncompressed.toUtf8());
#else
                    deserialized = deserializer.deserializeValue(uncompressed.toUtf8().toStdString());
#endif
                    JD_GENERAL_PROFILING_END_BLOCK;
                }
                else
                {
                    JD_GENERAL_PROFILING_END_BLOCK;
                    JD_GENERAL_PROFILING_NONSCOPED_BLOCK("import json", JD_COLOR_STAGE_6);
#ifdef JD_USE_QJSON
                    jsonDocument = QJsonDocument::fromJson(fileData);
#else
                    deserialized = deserializer.deserializeValue(fileData.toStdString());
#endif
                    JD_GENERAL_PROFILING_END_BLOCK;
                }
            }
            else
            {
                JD_GENERAL_PROFILING_NONSCOPED_BLOCK("import json", JD_COLOR_STAGE_6);
#ifdef JD_USE_QJSON
                jsonDocument = QJsonDocument::fromJson(fileData);
#else
                deserialized = deserializer.deserializeValue(fileData.toStdString());
#endif
                JD_GENERAL_PROFILING_END_BLOCK;
            }


#ifdef JD_USE_QJSON
            // Check if the JSON document is an array
            if (jsonDocument.isArray()) 
#else
            if (deserialized.isArray())
#endif
            {
                JD_GENERAL_PROFILING_NONSCOPED_BLOCK("QJsonArray to std::vector", JD_COLOR_STAGE_6);
#ifdef JD_USE_QJSON
                QJsonArray jsonArray = jsonDocument.array();

                // Iterate through the array and add QJsonObjects to the vector
                jsonsOut.reserve(jsonArray.size());
                for (const auto& jsonValue : jsonArray) {
                    if (jsonValue.isObject()) {
                        jsonsOut.emplace_back(jsonValue.toObject());
                    }
                }
#else
                // mesure start time using chrono
                std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
                // deserialized.getArray(jsonsOut); // Old slow version
                {
                    JsonValue::JsonVariantType& variant = deserialized.getVariant();
                    jsonsOut = std::move(std::get<JsonArray>(variant));
                }

                // mesure end time using chrono
                std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
                // mesure time difference and convert to ms
                double time1 = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000.0;
                // print time difference
                std::cout << "Time difference = " << time1 << "[ms]" << std::endl;

                JsonArray testArray;
                begin = std::chrono::steady_clock::now();
                testArray = std::move(jsonsOut);
				end = std::chrono::steady_clock::now();
                double time2 = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000.0;
                // print time difference
                std::cout << "Time difference = " << time2 << "[ms]" << std::endl;
#endif
                JD_GENERAL_PROFILING_END_BLOCK;
                return true;
            }
            else {
                JD_CONSOLE("bool JDManagerFileSystem::readJsonFile("
                    << "vector<QJsonObject>&, "
                    << directory << ", "
                    << fileName << ", "
                    << fileEnding << ", "
                    << "zipFormat=" << (zipFormat ? "true" : "false") << ", "
                    << "lockedRead=" << (lockedRead ? "true" : "false")
                    << ") JSON document from file : " << directory + "\\" + fileName + fileEnding << " is not an array\n"); 
                return false;
            }

            return false;
        }

#ifdef JD_USE_QJSON
        bool JDManagerFileSystem::readJsonFile(
            QJsonObject& objOut,
            const std::string& directory,
            const std::string& fileName,
            const std::string& fileEnding,
            bool zipFormat,
            bool lockedRead) const
#else
        bool JDManagerFileSystem::readJsonFile(
            JsonValue& objOut,
            const std::string& directory,
            const std::string& fileName,
            const std::string& fileEnding,
            bool zipFormat,
            bool lockedRead) const
#endif
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_5);
            QByteArray fileData;
            if (!readFile(fileData, directory, fileName, fileEnding, lockedRead))
            {
                return false;
            }
#ifdef JD_USE_QJSON
            QJsonParseError jsonError;
            jsonError.error = QJsonParseError::NoError;

            QJsonDocument document;
#else
            JsonDeserializer deserializer;
            JsonValue deserialized;
#endif
            if (zipFormat)
            {
                JD_GENERAL_PROFILING_NONSCOPED_BLOCK("uncompressing data", JD_COLOR_STAGE_6);
                QString uncompressed;
                if (StringZipper::decompressString(fileData, uncompressed))
                {
                    JD_GENERAL_PROFILING_END_BLOCK;
                    JD_GENERAL_PROFILING_NONSCOPED_BLOCK("import json", JD_COLOR_STAGE_6);
#ifdef JD_USE_QJSON
                    document = QJsonDocument::fromJson(uncompressed.toUtf8());
#else
                    deserialized = deserializer.deserializeValue(uncompressed.toUtf8().toStdString());
#endif
                    JD_GENERAL_PROFILING_END_BLOCK;
                }
                else
                {
                    JD_GENERAL_PROFILING_END_BLOCK;
                    JD_GENERAL_PROFILING_NONSCOPED_BLOCK("import json", JD_COLOR_STAGE_6);
#ifdef JD_USE_QJSON
                    document = QJsonDocument::fromJson(fileData);
#else
                    deserialized = deserializer.deserializeValue(fileData.toStdString());
#endif
                    JD_GENERAL_PROFILING_END_BLOCK;
                }
            }
            else
            {
                JD_GENERAL_PROFILING_NONSCOPED_BLOCK("import json", JD_COLOR_STAGE_6);
#ifdef JD_USE_QJSON
                document = QJsonDocument::fromJson(fileData, &jsonError);
#else
                deserialized = deserializer.deserializeValue(fileData.toStdString());
#endif
                JD_GENERAL_PROFILING_END_BLOCK;
            }
#ifdef JD_USE_QJSON
            if (jsonError.error != QJsonParseError::NoError)
            {
                JD_CONSOLE("bool JDManagerFileSystem::readJsonFile("
                    << "QJsonObject&, "
                    << directory << ", "
                    << fileName << ", "
                    << fileEnding << ", "
                    << "zipFormat=" << (zipFormat ? "true" : "false") << ", "
                    << "lockedRead=" << (lockedRead ? "true" : "false")
                    << ") Can't read Jsonfile: " << jsonError.errorString().toStdString().c_str() << "\n");
                return false;
            }
#else

#endif
#ifdef JD_USE_QJSON
            if (document.isObject())
            {
                objOut = document.object();
                return true;
            }
#else
            if (deserialized.isObject())
            {
                //objOut = std::move(deserialized);
                objOut = deserialized;
                return true;
            }
#endif

            return false;
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
                    JD_CONSOLE("bool JDManagerFileSystem::readFile("
                        << "QByteArray&, "
                        << directory << ", "
                        << fileName << ", "
                        << fileEnding << ", "
                        << "lockedRead=" << (lockedRead ? "true" : "false")
                        << ") Can't lock file\n");
                    return false;
                }
            }


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
                JD_CONSOLE("bool JDManagerFileSystem::readFile("
                    << "QByteArray&, "
                    << directory << ", "
                    << fileName << ", "
                    << fileEnding << ", "
                    << "lockedRead=" << (lockedRead ? "true" : "false")
                    << ") Can't open file: " << filePath.c_str() << "\n");
                if (lockedRead)
                    unlockFile();
                return false;
            }
            JDFILE_IO_PROFILING_END_BLOCK;
            JDFILE_IO_PROFILING_NONSCOPED_BLOCK("read from file", JD_COLOR_STAGE_6);
            DWORD fileSize = GetFileSize(fileHandle, nullptr);
            if (fileSize == INVALID_FILE_SIZE) {
                CloseHandle(fileHandle);
                JD_CONSOLE("bool JDManagerFileSystem::readFile("
                    << "QByteArray&, "
                    << directory << ", "
                    << fileName << ", "
                    << fileEnding << ", "
                    << "lockedRead=" << (lockedRead ? "true" : "false")
                    << ") Can't get filesize of: " << filePath.c_str() << "\n");
                JDFILE_IO_PROFILING_END_BLOCK;
                if (lockedRead)
                    unlockFile();
                return false;
            }
            fileDataOut.resize(fileSize);
            //std::string content(fileSize, '\0');
            DWORD bytesRead;
            BOOL readResult = ReadFile(
                fileHandle,
                &fileDataOut.data()[0],
                fileSize,
                &bytesRead,
                nullptr
            );

            CloseHandle(fileHandle);
            JDFILE_IO_PROFILING_END_BLOCK;
            if (lockedRead)
                unlockFile();

            if (!readResult) {
                JD_CONSOLE("bool JDManagerFileSystem::readFile("
                    << "QByteArray&, "
                    << directory << ", "
                    << fileName << ", "
                    << fileEnding << ", "
                    << "lockedRead=" << (lockedRead ? "true" : "false")
                    << ") Can't read file: " << filePath.c_str() << "\n");
            }
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
                    JD_CONSOLE("bool JDManagerFileSystem::writeFile("
                        << "QByteArray&, "
                        << directory << ", "
                        << fileName << ", "
                        << fileEnding << ", "
                        << "lockedRead=" << (lockedRead ? "true" : "false")
                        << ") Can't lock file\n");
                    return false;
                }
            }

            
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
                JD_CONSOLE("bool JDManagerFileSystem::writeFile("
                    << "QByteArray&, "
                    << directory << ", "
                    << fileName << ", "
                    << fileEnding << ", "
                    << "lockedRead=" << (lockedRead ? "true" : "false")
                    << ") Could not open file " << filePath << " for writing\n");
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
                JD_CONSOLE("bool JDManagerFileSystem::writeFile("
                    << "QByteArray&, "
                    << directory << ", "
                    << fileName << ", "
                    << fileEnding << ", "
                    << "lockedRead=" << (lockedRead ? "true" : "false")
                    << ") Could not write to file " << filePath << "\n");
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
                if (file.open(QIODevice::WriteOnly))
                {
					file.close();
					return true;
				}
                else
                {
					JD_CONSOLE("bool JDManagerFileSystem::makeDatabaseFiles() Can't create database file: " << m_manager.getDatabaseFilePath().c_str() << "\n");
                    return false;
                }
            }
            return true;
        }

        bool JDManagerFileSystem::deleteDir(const std::string& dir) const
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            int ret = SystemCommand::execute("rd /s /q \"" + dir + "\"");
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

        ManagedFileChangeWatcher& JDManagerFileSystem::getDatabaseFileWatcher()
        {
            return m_fileWatcher;
        }
        void JDManagerFileSystem::restartDatabaseFileWatcher()
        {
            m_fileWatcher.setup(m_manager.getDatabaseFilePath());
        }
        
        void JDManagerFileSystem::update()
        {
            if (m_fileWatcher.hasFileChanged())
            {
                m_manager.getSignals().databaseFileChanged.emitSignal();
                m_fileWatcher.clearHasFileChanged();
            }
        }
    }
}