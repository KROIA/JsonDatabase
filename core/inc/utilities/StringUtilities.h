#pragma once

#include "JD_base.h"
#include <QDateTime>

namespace JsonDatabase
{
    namespace Utilities
    {
        std::string JSONDATABASE_EXPORT replaceForwardSlashesWithBackslashes(const std::string& input);
        std::string JSONDATABASE_EXPORT generateRandomString(int length);


        std::string JSONDATABASE_EXPORT qDateToString(const QDate& date);
        std::string JSONDATABASE_EXPORT qTimeToString(const QTime& time);

        QDate JSONDATABASE_EXPORT stringToQDate(const std::string& date);
        QTime JSONDATABASE_EXPORT stringToQTime(const std::string& time);
    }
}