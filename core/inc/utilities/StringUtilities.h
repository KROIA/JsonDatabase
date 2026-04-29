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
        // Faster alternative for QDate::fromString with "dd.MM.yyyy" format
        QDate JSON_DATABASE_API fastStringToQDate(const std::string& dateStr);


        QTime JSON_DATABASE_API stringToQTime(const std::string& time);
        // Faster alternative for QTime::fromString with "hh:mm:ss:zzz" format
        QTime JSON_DATABASE_API fastStringToQTime(const std::string& timeStr);

        std::string JSON_DATABASE_API wstrToStr(const std::wstring& wstr);
        std::wstring JSON_DATABASE_API strToWstr(const std::string& str);
    }
}
