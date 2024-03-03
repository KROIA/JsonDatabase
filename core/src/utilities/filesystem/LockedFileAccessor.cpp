#include "utilities/filesystem/LockedFileAccessor.h"
#include "manager/async/WorkProgress.h"
#include "utilities/filesystem/StringZipper.h"


#include "Json/JsonValue.h"
#include "Json/JsonDeserializer.h"
#include "Json/JsonSerializer.h"


#include <iostream>

namespace JsonDatabase
{
	namespace Internal
	{

		LockedFileAccessor::LockedFileAccessor(
			const std::string& directory,
			const std::string& name,
			const std::string& endig)
			: m_fileLock(nullptr)
			, m_directory(directory)
			, m_name(name)
			, m_ending(endig)
			, m_useZipFormat(false)
			, m_progress(nullptr)
		{

		}
		LockedFileAccessor::~LockedFileAccessor()
		{
            unlock();
		}

		LockedFileAccessor::Error LockedFileAccessor::lock(AccessMode mode)
		{
			LockedFileAccessor::Error errorOut = LockedFileAccessor::Error::none;
			FileReadWriteLock::Access direction = (FileReadWriteLock::Access)mode;
			if (m_fileLock)
			{
				JD_CONSOLE("LockedFileAccessor::lock("
					<< FileReadWriteLock::accessTypeToString(direction)
					<< ") Lock already aquired\n");
				errorOut = Error::fileLock_alreadyLocked;
				return errorOut;
			}

			m_fileLock = new FileReadWriteLock(m_directory, getFullFileName());

			FileLock::Error lockErr;
			bool wasLockedForWritingByOther = false;
			if (!m_fileLock->lock(direction, wasLockedForWritingByOther, lockErr))
			{
				JD_CONSOLE("bool LockedFileAccessor::lock("
					<< FileReadWriteLock::accessTypeToString(direction)
					<< ", bool) Can't aquire lock for: " 
					<< getFullFilePath() <<"\n");
				delete m_fileLock;
				m_fileLock = nullptr;
				errorOut = (Error)lockErr;
			}
			return errorOut;
		}
		LockedFileAccessor::Error LockedFileAccessor::lock(AccessMode mode, unsigned int timeoutMillis)
		{
			LockedFileAccessor::Error errorOut = LockedFileAccessor::Error::none;
			FileReadWriteLock::Access direction = (FileReadWriteLock::Access)mode;
			if (m_fileLock)
			{
				JD_CONSOLE("bool LockedFileAccessor::lock("
					<< FileReadWriteLock::accessTypeToString(direction)
					<< ", timeout=" << timeoutMillis << "ms) Lock already aquired\n");
				errorOut = Error::fileLock_alreadyLocked;
				return errorOut;
			}

			m_fileLock = new FileReadWriteLock(m_directory, getFullFileName());
			FileLock::Error lockErr;
			bool wasLockedForWritingByOther = false;
			if (!m_fileLock->lock(direction, timeoutMillis, wasLockedForWritingByOther, lockErr))
			{
				JD_CONSOLE("LockedFileAccessor::lock("
					<< FileReadWriteLock::accessTypeToString(direction)
					<< ", timeout=" << timeoutMillis << "ms) Error while trying to aquire file lock for: " 
					<< getFullFilePath() << "\n"
					<< "Error: " << FileLock::getErrorStr(lockErr));
				delete m_fileLock;
				m_fileLock = nullptr;
				errorOut = (Error)lockErr;
			}
			return errorOut;
		}
		LockedFileAccessor::Error LockedFileAccessor::unlock()
		{
			LockedFileAccessor::Error errorOut = LockedFileAccessor::Error::none;
			if (!m_fileLock)
				return errorOut;
			FileLock::Error lockErr;
			m_fileLock->unlock(lockErr);
			errorOut = (Error)lockErr;
			delete m_fileLock;
			m_fileLock = nullptr;

			return errorOut;
		}
        bool LockedFileAccessor::isLocked() const
        {
            if (!m_fileLock) return false;
            return m_fileLock->isLocked();
        }

		void LockedFileAccessor::useZipFormat(bool useZipFormat)
		{
			m_useZipFormat = useZipFormat;
		}
		bool LockedFileAccessor::useZipFormat() const
		{
			return m_useZipFormat;
		}

		void LockedFileAccessor::setProgress(Internal::WorkProgress* progress)
		{
			m_progress = progress;
		}
		Internal::WorkProgress* LockedFileAccessor::progress() const
		{
			return m_progress;
		}

        std::string LockedFileAccessor::getFullFilePath() const
        {
            return m_directory + "\\" + m_name + m_ending;
        }
        std::string LockedFileAccessor::getFullFileName() const
        {
            return m_name + m_ending;
        }



        LockedFileAccessor::Error LockedFileAccessor::writeJsonFile(const JsonArray& jsons) const
        {
            
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_5);
            if(!isLocked())
            {
                JD_CONSOLE("LockedFileAccessor::writeJsonFile(const JsonArray&) File is not locked\n");
				return Error::fileLock_notAquired;
			}

            double progressScalar = 0;
            if (m_progress)
                progressScalar = m_progress->getScalar();



            JD_GENERAL_PROFILING_NONSCOPED_BLOCK("toJson", JD_COLOR_STAGE_6);
            QByteArray data;
            if (m_progress)
            {
                m_progress->setComment("Export Json objects");
                m_progress->startNewSubProgress(progressScalar * 0.9);
            }
            JsonSerializer serializer;
            serializer.enableTabs(false);
            serializer.enableNewLinesInObjects(false);
            serializer.enableNewLineAfterObject(true);
            serializer.enableSpaces(false);
            std::string bufferStr = serializer.serializeArray(jsons, m_progress);
            data = QByteArray::fromStdString(bufferStr);
            JD_GENERAL_PROFILING_END_BLOCK;

           // m_fileWatcher.pause();
            // Write the JSON data to the file
            if (m_progress)
            {
                m_progress->setComment("Write File");
                m_progress->startNewSubProgress(progressScalar * 0.1);
            }

            if (m_useZipFormat)
            {
                JD_GENERAL_PROFILING_NONSCOPED_BLOCK("compressing data", JD_COLOR_STAGE_6);
                QByteArray fileData;
                StringZipper::compressString(data, fileData);
                JD_GENERAL_PROFILING_END_BLOCK;

                return writeFile(fileData);
            }
            else
            {
                return writeFile(data);
            }
           // m_fileWatcher.unpause();
            return Error::file_cantWriteFile;
        }


        LockedFileAccessor::Error LockedFileAccessor::writeJsonFile(const JsonObject& json) const
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_5);

            if (!isLocked())
            {
                JD_CONSOLE("LockedFileAccessor::writeJsonFile(const JsonValue&) File is not locked\n");
                return Error::fileLock_notAquired;
            }

            double progressScalar = 0;
            if (m_progress)
            {
                progressScalar = m_progress->getScalar();
            }

            JD_GENERAL_PROFILING_NONSCOPED_BLOCK("toJson", JD_COLOR_STAGE_6);
            QByteArray data;
            if (m_progress)
            {
                m_progress->setComment("Export Json objects");
                m_progress->startNewSubProgress(progressScalar * 0.9);
            }

            JsonSerializer serializer;
            serializer.enableTabs(false);
            serializer.enableNewLinesInObjects(false);
            serializer.enableNewLineAfterObject(true);
            serializer.enableSpaces(false);
            std::string fileBuffer = serializer.serializeObject(json);
            data = QByteArray::fromStdString(fileBuffer);

            JD_GENERAL_PROFILING_END_BLOCK;
            if (m_progress)
            {
                m_progress->setComment("Write File");
                m_progress->startNewSubProgress(progressScalar * 0.1);
            }

            // Write the JSON data to the file
            if (m_useZipFormat)
            {
                JD_GENERAL_PROFILING_NONSCOPED_BLOCK("compressing data", JD_COLOR_STAGE_6);
                QByteArray fileData;
                StringZipper::compressString(data, fileData);
                JD_GENERAL_PROFILING_END_BLOCK;

                return writeFile(fileData);
            }
            else
            {
                return writeFile(data);
            }
            return Error::file_cantWriteFile;
        }


        LockedFileAccessor::Error LockedFileAccessor::readJsonFile(JsonArray& jsonsOut) const
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_5);

            if (!isLocked())
            {
                JD_CONSOLE("LockedFileAccessor::readJsonFile(JsonArray&) File is not locked\n");
                return Error::fileLock_notAquired;
            }

            double progressScalar = 0;
            if (m_progress)
            {
                progressScalar = m_progress->getScalar();
                m_progress->startNewSubProgress(progressScalar * 0.1);
                m_progress->setComment("Read File");
            }

            QByteArray fileData;
            Error errorOut;
            if ((errorOut = readFile(fileData)) != Error::none)
            {
                return errorOut;
            }

            // Parse the JSON data
            JsonDeserializer deserializer;
            JsonArray deserialized{};
            if (m_progress)
            {
                m_progress->startNewSubProgress(progressScalar * 0.9);
                m_progress->setComment("Import Json Objects");
            }
            if (m_useZipFormat)
            {
                JD_GENERAL_PROFILING_NONSCOPED_BLOCK("uncompressing data", JD_COLOR_STAGE_6);
                QString uncompressed;
                if (StringZipper::decompressString(fileData, uncompressed))
                {
                    JD_GENERAL_PROFILING_END_BLOCK;
                    JD_GENERAL_PROFILING_NONSCOPED_BLOCK("import json", JD_COLOR_STAGE_6);
                    std::string converted = uncompressed.toUtf8().toStdString();
                    if (m_progress)
                        deserialized = deserializer.deserializeArray(converted, m_progress);
                    else
                        deserialized = deserializer.deserializeArray(converted);
                    JD_GENERAL_PROFILING_END_BLOCK;
                }
                else
                {
                    JD_GENERAL_PROFILING_END_BLOCK;
                    JD_GENERAL_PROFILING_NONSCOPED_BLOCK("import json", JD_COLOR_STAGE_6);
                    std::string converted = fileData.toStdString();
                    if (m_progress)
                        deserialized = deserializer.deserializeArray(converted, m_progress);
                    else
                        deserialized = deserializer.deserializeArray(converted);
                    JD_GENERAL_PROFILING_END_BLOCK;
                }
            }
            else
            {
                JD_GENERAL_PROFILING_NONSCOPED_BLOCK("import json", JD_COLOR_STAGE_6);
                std::string str = std::move(fileData.toStdString());
                bool success = false;
                if (m_progress)
                {

                   // double timeMs = 0;
                    //int iterations = 100;
                    //for (int i = 0; i < iterations; ++i)
                    //{
                      //  JsonArray arr;
                        //auto start = std::chrono::steady_clock::now();
             
                    success = deserializer.deserializeArray(str, deserialized, m_progress);
                    

                        
                        
                        //deserializer.deserializeArray(str, arr);
                        //auto end = std::chrono::steady_clock::now();
                        
                        //timeMs += (double)std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
                    //}
                    //timeMs /= (iterations * 1000000.0);

                    //std::cout << "Elapsed time in milliseconds: "
                     //   << std::setprecision(9) << timeMs
                       // << " ms\n";
                }
                else
                    success = deserializer.deserializeArray(str, deserialized);
               
                JD_GENERAL_PROFILING_END_BLOCK;
            }

            JD_GENERAL_PROFILING_BLOCK("Move JsonArray", JD_COLOR_STAGE_6);
            {
                jsonsOut = std::move(deserialized);
            }
            return Error::none;
        }
        LockedFileAccessor::Error LockedFileAccessor::readJsonFile(JsonObject& objOut) const
        {
            if (!isLocked())
            {
                JD_CONSOLE("LockedFileAccessor::readJsonFile(JsonValue&) File is not locked\n");
                return Error::fileLock_notAquired;
            }
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_5);
            double progressScalar = 0;
            if (m_progress)
            {
                progressScalar = m_progress->getScalar();
                m_progress->startNewSubProgress(progressScalar * 0.1);
                m_progress->setComment("Read File");
            }

            QByteArray fileData;
            Error errorOut;
            if ((errorOut = readFile(fileData)) != Error::none)
            {
                return errorOut;
            }
            JsonDeserializer deserializer;
            JsonObject deserialized{};
            if (m_progress)
            {
                m_progress->startNewSubProgress(progressScalar * 0.9);
                m_progress->setComment("Import Json Objects");
            }
            if (m_useZipFormat)
            {
                JD_GENERAL_PROFILING_NONSCOPED_BLOCK("uncompressing data", JD_COLOR_STAGE_6);
                QString uncompressed;
                if (StringZipper::decompressString(fileData, uncompressed))
                {
                    JD_GENERAL_PROFILING_END_BLOCK;
                    JD_GENERAL_PROFILING_NONSCOPED_BLOCK("import json", JD_COLOR_STAGE_6);
                    std::string bufferStr = uncompressed.toUtf8().toStdString();
                    if (m_progress)
                        deserializer.deserializeObject(bufferStr, deserialized, m_progress);
                    else
                        deserializer.deserializeObject(bufferStr, deserialized);
                    JD_GENERAL_PROFILING_END_BLOCK;
                }
                else
                {
                    JD_GENERAL_PROFILING_END_BLOCK;
                    JD_GENERAL_PROFILING_NONSCOPED_BLOCK("import json", JD_COLOR_STAGE_6);
                    std::string bufferStr = fileData.toStdString();
                    if (m_progress)
                        deserializer.deserializeObject(bufferStr, deserialized, m_progress);
                    else
                        deserializer.deserializeObject(bufferStr, deserialized);
                    JD_GENERAL_PROFILING_END_BLOCK;
                }
            }
            else
            {
                JD_GENERAL_PROFILING_NONSCOPED_BLOCK("import json", JD_COLOR_STAGE_6);
                std::string bufferStr = fileData.toStdString();
                if (m_progress)
                    deserializer.deserializeObject(bufferStr, deserialized, m_progress);
                else
                    deserializer.deserializeObject(bufferStr, deserialized);
                JD_GENERAL_PROFILING_END_BLOCK;
            }
            objOut = std::move(deserialized);
            return Error::none;
        }

        LockedFileAccessor::Error LockedFileAccessor::readFile(QByteArray& fileDataOut) const
        {
            if (!isLocked())
            {
                JD_CONSOLE("LockedFileAccessor::readFile(QByteArray&) File is not locked\n");
                return Error::fileLock_notAquired;
            }
            return readFile_internal(fileDataOut);
        }
        LockedFileAccessor::Error LockedFileAccessor::writeFile(const QByteArray& fileData) const
        {
            if (!isLocked())
            {
                JD_CONSOLE("LockedFileAccessor::writeFile(QByteArray&) File is not locked\n");
                return Error::fileLock_notAquired;
            }
            return writeFile_internal(fileData);
        }

        const std::string& LockedFileAccessor::getErrorStr(Error err)
        {
#define JD_CASE_RETURN_ERROR_STR(x) case x: { static const std::string msg = "LockedFileAccessor::"#x; return msg; }
            
            switch (err)
            {
                JD_CASE_RETURN_ERROR_STR(Error::none);

                JD_CASE_RETURN_ERROR_STR(Error::fileLock_unableToCreateOrOpenLockFile);
                JD_CASE_RETURN_ERROR_STR(Error::fileLock_unableToDeleteLockFile);
                JD_CASE_RETURN_ERROR_STR(Error::fileLock_unableToLock);
                JD_CASE_RETURN_ERROR_STR(Error::fileLock_alreadyLocked);
                JD_CASE_RETURN_ERROR_STR(Error::fileLock_alreadyLockedForReading);
                JD_CASE_RETURN_ERROR_STR(Error::fileLock_alreadyLockedForWritingByOther);
                JD_CASE_RETURN_ERROR_STR(Error::fileLock_alreadyUnlocked);
                JD_CASE_RETURN_ERROR_STR(Error::fileLock_notAquired);

                JD_CASE_RETURN_ERROR_STR(Error::json_parseError);
                JD_CASE_RETURN_ERROR_STR(Error::json_isNotAnObject);
                JD_CASE_RETURN_ERROR_STR(Error::json_isNotAnArray);

                JD_CASE_RETURN_ERROR_STR(Error::file_cantOpenFileForRead);
                JD_CASE_RETURN_ERROR_STR(Error::file_cantOpenFileForWrite);
                JD_CASE_RETURN_ERROR_STR(Error::file_invalidFileSize);
                JD_CASE_RETURN_ERROR_STR(Error::file_cantReadFile);
                JD_CASE_RETURN_ERROR_STR(Error::file_cantWriteFile);
                JD_CASE_RETURN_ERROR_STR(Error::file_cantVerifyFileContents);
            }
            static std::string unknown;
            unknown = "Unknown FileLock Error: " + std::to_string((int)err);
            return unknown;

        }

        LockedFileAccessor::Error LockedFileAccessor::readFile_internal(QByteArray& fileDataOut) const
        {
            JDFILE_IO_PROFILING_FUNCTION(JD_COLOR_STAGE_6);
            JDFILE_IO_PROFILING_NONSCOPED_BLOCK("open file", JD_COLOR_STAGE_7);
            std::string filePath = getFullFilePath();
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
                JD_CONSOLE("bool LockedFileAccessor::readFile("
                    << "QByteArray&) Can't open file: " << filePath.c_str() << "\n");
                Error err;
                return Error::file_cantOpenFileForRead;
            }
            JDFILE_IO_PROFILING_END_BLOCK;
            JDFILE_IO_PROFILING_NONSCOPED_BLOCK("read from file", JD_COLOR_STAGE_7);
            DWORD fileSize = GetFileSize(fileHandle, nullptr);
            if (fileSize == INVALID_FILE_SIZE) {
                CloseHandle(fileHandle);
                JD_CONSOLE("bool LockedFileAccessor::readFile("
                    << "QByteArray&) Can't get filesize of: " << filePath.c_str() << "\n");
                JDFILE_IO_PROFILING_END_BLOCK;
                Error err;                
                return Error::file_invalidFileSize;;
            }
            fileDataOut.resize(fileSize);
            DWORD totalBytesRead = 0;
            BOOL readResult;

            if (m_progress)
            {
                size_t chunkCount = 100;
                size_t chunkSize = fileSize / chunkCount;
                readResult = true;
                DWORD startIndex = 0;
                for (size_t i = 0; i < chunkCount; ++i)
                {
                    DWORD size = chunkSize;
                    if (i == chunkCount - 1)
                        size = fileSize - totalBytesRead;
                    if (totalBytesRead + size > fileSize)
                        size = fileSize - totalBytesRead;

                    DWORD bytesRead = 0;
                    readResult &= ReadFile(
                        fileHandle,
                        &fileDataOut.data()[startIndex],
                        size,
                        &bytesRead,
                        nullptr
                    );
                    startIndex += size;
                    totalBytesRead += bytesRead;
                    double progressPercent = (double)totalBytesRead / (double)fileSize;
                    m_progress->setProgress(progressPercent);
                }
            }
            else
            {
                readResult = ReadFile(
                    fileHandle,
                    &fileDataOut.data()[0],
                    fileSize,
                    &totalBytesRead,
                    nullptr
                );
            }

            CloseHandle(fileHandle);
            JDFILE_IO_PROFILING_END_BLOCK;

            if (!readResult) {
                JD_CONSOLE("bool LockedFileAccessor::readFile("
                    << "QByteArray&) Can't read file: " << filePath.c_str() << "\n");
                return Error::file_cantReadFile;
            }
            return Error::none;
        }
        LockedFileAccessor::Error LockedFileAccessor::writeFile_internal(const QByteArray& fileData) const
        {
            JDFILE_IO_PROFILING_FUNCTION(JD_COLOR_STAGE_6);

            double progressScalar = 0;
            if (m_progress)
            {
                progressScalar = m_progress->getScalar();
            }

            // Open the file for writing
            JDFILE_IO_PROFILING_BLOCK("open file", JD_COLOR_STAGE_7);
            std::string filePath = getFullFilePath();
            HANDLE fileHandle = CreateFile(
                filePath.c_str(),
                GENERIC_WRITE,
                0,
                nullptr,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                nullptr
            );
            JDFILE_IO_PROFILING_END_BLOCK;
            if (fileHandle == INVALID_HANDLE_VALUE) {
                JD_CONSOLE("bool LockedFileAccessor::writeFile("
                    << "QByteArray&) Could not open file " << filePath << " for writing\n");
                // Error opening file
                Error err;

                return Error::file_cantOpenFileForWrite;
            }

            JDFILE_IO_PROFILING_BLOCK("write to file", JD_COLOR_STAGE_7);
            // Write the content to the file
            BOOL writeResult;
            DWORD totalBytesWritten = 0;

            if (m_progress)
            {
                m_progress->startNewSubProgress(progressScalar * 0.5);
                size_t chunkCount = 100;
                size_t chunkSize = fileData.size() / chunkCount;
                const char* start = fileData.constData();
                writeResult = true;
                for (size_t i = 0; i < chunkCount; ++i)
                {
                    DWORD size = chunkSize;
                    if (i == chunkCount - 1)
                        size = fileData.size() - totalBytesWritten;
                    if (totalBytesWritten + size > (DWORD)fileData.size())
                        size = fileData.size() - totalBytesWritten;

                    DWORD bytesWritten = 0;
                    writeResult &= WriteFile(
                        fileHandle,
                        start,
                        size,
                        &bytesWritten,
                        nullptr
                    );
                    start += size;
                    totalBytesWritten += bytesWritten;
                    double progressPercent = (double)totalBytesWritten / (double)fileData.size();
                    m_progress->setProgress(progressPercent);
                }
            }
            else
            {
                writeResult = WriteFile(
                    fileHandle,
                    fileData.constData(),
                    fileData.size(),
                    &totalBytesWritten,
                    nullptr
                );
            }

            // Close the file handle
            CloseHandle(fileHandle);

            if (!writeResult) {
                // Error writing to file
                JD_CONSOLE("bool LockedFileAccessor::writeFile(QByteArray&) Could not write to file " << filePath << "\n");
                return Error::file_cantWriteFile;
            }

            JDFILE_IO_PROFILING_END_BLOCK;

            JDFILE_IO_PROFILING_BLOCK("Verify file content", JD_COLOR_STAGE_7);
            // Verify file content
            QByteArray readFileContent;
            Error err1;
            if (m_progress)
            {
                m_progress->setComment("Verifying file content");
                m_progress->startNewSubProgress(progressScalar * 0.5);
            }
            if ((err1 = readFile(readFileContent)) != Error::none)
            {
                // Can't verify read
                return Error::file_cantVerifyFileContents;
            }

            // Compare file content
            if (readFileContent != fileData)
            {
                // File content is not the same
                return Error::file_cantVerifyFileContents;
            }
            JDFILE_IO_PROFILING_END_BLOCK;
            return Error::none;
        }
	}
}