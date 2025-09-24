#pragma once

#include "JsonDatabase_base.h"
#include <QDateTime>

namespace JsonDatabase
{
    namespace Utilities
    {
        std::string JSON_DATABASE_API replaceForwardSlashesWithBackslashes(const std::string& input);
        std::string JSON_DATABASE_API generateRandomString(int length);


        std::string JSON_DATABASE_API qDateToString(const QDate& date);
        std::string JSON_DATABASE_API qTimeToString(const QTime& time);

        QDate JSON_DATABASE_API stringToQDate(const std::string& date);
        QTime JSON_DATABASE_API stringToQTime(const std::string& time);

        std::string JSON_DATABASE_API wstrToStr(const std::wstring& wstr);
        std::wstring JSON_DATABASE_API strToWstr(const std::string& str);
    }
}
