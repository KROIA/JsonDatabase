#pragma once

#include "JD_base.h"
#include <QString>
#include <string>

namespace JsonDatabase
{
    namespace Internal
    {
        namespace StringZipper
        {
            extern void compressString(const QString& inputString, QByteArray& compressedData);
            extern bool decompressString(QByteArray compressedData, QString& outputString);
        }
    }
}