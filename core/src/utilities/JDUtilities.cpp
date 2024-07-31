#include "utilities/JDUtilities.h"
#include <QFile>
#include <QCryptographicHash>
#include <QByteArray>
#include <QString>

namespace JsonDatabase
{
	namespace Utilities
	{
		std::string getLastErrorString(DWORD error)
		{
			std::string errorString;
			LPSTR messageBuffer = nullptr;
			size_t size = FormatMessageA(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				error,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPSTR)&messageBuffer,
				0,
				NULL);
			errorString = std::string(messageBuffer, size);
			LocalFree(messageBuffer);
			return errorString;
		}

		std::string JSON_DATABASE_EXPORT calculateMD5Hash(const std::string& filePath, Log::LogObject* logger, bool& success)
		{
			QFile file(filePath.c_str());
			if (!file.open(QIODevice::ReadOnly)) {
				if(logger)logger->logWarning("Could not open file for reading: " + filePath);
				success = false;
				return "";
			}

			QCryptographicHash hash(QCryptographicHash::Md5);
			if (!hash.addData(&file)) {
				if(logger)logger->logWarning("Could not add file data to hash: " + filePath);
				success = false;
				return "";
			}

			QByteArray result = hash.result();
			file.close();
			success = true;
			return result.toHex().constData();
		}

		
	}
}