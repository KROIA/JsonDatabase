#include "utilities/StringUtilities.h"

namespace JsonDatabase
{
    namespace Utilities
    {
        std::string replaceForwardSlashesWithBackslashes(const std::string& input)
        {
            std::string result = input;
            size_t pos = 0;

            while ((pos = result.find('/', pos)) != std::string::npos)
            {
                result.replace(pos, 1, "\\");
                pos += 2; // Move past the double backslashes
            }

            return result;
        }
		std::string generateRandomString(int length)
		{
			std::string result;
			result.resize(length);
			// array of chars to choose from
			static const char alphanum[] =
				"0123456789"
				"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				"abcdefghijklmnopqrstuvwxyz";

			for (int i = 0; i < length; ++i)
			{
				result[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
			}
			return result;
		}

		std::string qDateToString(const QDate& date)
		{
			return date.toString("dd.MM.yyyy").toStdString();
		}
		std::string qTimeToString(const QTime& time)
		{
			return time.toString("hh:mm:ss").toStdString();
		}

		QDate stringToQDate(const std::string& date)
		{
			return QDate::fromString(QString::fromStdString(date), "dd.MM.yyyy");
		}
		QTime stringToQTime(const std::string& time)
		{
			return QTime::fromString(QString::fromStdString(time), "hh:mm:ss");
		}
    }
}