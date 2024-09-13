#pragma once

#include "JsonDatabase_base.h"
#include <QDateTime>

namespace JsonDatabase
{
    namespace Utilities
    {
        std::string JSON_DATABASE_EXPORT replaceForwardSlashesWithBackslashes(const std::string& input);
        std::string JSON_DATABASE_EXPORT generateRandomString(int length);


        std::string JSON_DATABASE_EXPORT qDateToString(const QDate& date);
        std::string JSON_DATABASE_EXPORT qTimeToString(const QTime& time);

        QDate JSON_DATABASE_EXPORT stringToQDate(const std::string& date);
        QTime JSON_DATABASE_EXPORT stringToQTime(const std::string& time);

        std::string JSON_DATABASE_EXPORT wstrToStr(const std::wstring& wstr);
        std::wstring JSON_DATABASE_EXPORT strToWstr(const std::string& str);
    }
}
