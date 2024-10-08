#include "utilities/StringUtilities.h"

namespace JsonDatabase
{
    namespace Utilities
    {
        std::string replaceForwardSlashesWithBackslashes(const std::string& input)
        {
			return input;
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

			// create a random number generator
			srand((unsigned int)time(NULL));

			// generate random string
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
			return time.toString("hh:mm:ss:zzz").toStdString();
		}

		QDate stringToQDate(const std::string& date)
		{
			return QDate::fromString(QString::fromStdString(date), "dd.MM.yyyy");
		}
		QTime stringToQTime(const std::string& time)
		{
			return QTime::fromString(QString::fromStdString(time), "hh:mm:ss:zzz");
		}

		std::string wstrToStr(const std::wstring& wstr)
		{
			std::string result(wstr.begin(), wstr.end());
			return result;			
		}
		std::wstring strToWstr(const std::string& str)
		{
			std::wstring result(str.begin(), str.end());
			return result;
		}
    }
}