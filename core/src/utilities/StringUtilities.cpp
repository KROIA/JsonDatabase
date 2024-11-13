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

		// Helper function to convert substrings to integers with validation
		static bool toInt(const std::string& str, int start, int length, int& out) {
			try {
				out = std::stoi(str.substr(start, length));
				return true;
			}
			catch (...) {
				return false;
			}
		}


		QDate stringToQDate(const std::string& date)
		{
			JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
			return QDate::fromString(date.c_str(), "dd.MM.yyyy");
		}


		// Faster alternative for QDate::fromString with "dd.MM.yyyy" format using std::string
		QDate fastStringToQDate(const std::string& dateStr) {
			JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
			if (dateStr.size() != 10 || dateStr[2] != '.' || dateStr[5] != '.') {
				return QDate(); // Invalid date format
			}

			int day, month, year;
			if (!toInt(dateStr, 0, 2, day) || day < 1 || day > 31) return QDate();
			if (!toInt(dateStr, 3, 2, month) || month < 1 || month > 12) return QDate();
			if (!toInt(dateStr, 6, 4, year) || year < 1) return QDate();

			QDate date(year, month, day);
			return date.isValid() ? date : QDate();  // Validate date to handle cases like Feb 30
		}

		

		
		QTime stringToQTime(const std::string& time)
		{
			JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
			return QTime::fromString(time.c_str(), "hh:mm:ss:zzz");
		}
		// Faster alternative for QTime::fromString with "hh:mm:ss:zzz" format using std::string
		QTime fastStringToQTime(const std::string& timeStr) {
			JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
			if (timeStr.size() != 12 || timeStr[2] != ':' || timeStr[5] != ':' || timeStr[8] != ':') {
				return QTime(); // Invalid time format
			}

			int hour, minute, second, millisecond;
			if (!toInt(timeStr, 0, 2, hour) || hour < 0 || hour > 23) return QTime();
			if (!toInt(timeStr, 3, 2, minute) || minute < 0 || minute > 59) return QTime();
			if (!toInt(timeStr, 6, 2, second) || second < 0 || second > 59) return QTime();
			if (!toInt(timeStr, 9, 3, millisecond) || millisecond < 0 || millisecond > 999) return QTime();

			return QTime(hour, minute, second, millisecond);
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