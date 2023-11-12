#include "utilities/filesystem/StringZipper.h"

namespace JsonDatabase
{
    namespace Internal
    {
        namespace StringZipper
        {
            void compressString(const QString& inputString, QByteArray& compressedData)
            {
                compressedData = qCompress(inputString.toUtf8(), -1);
            }
            bool decompressString(const QByteArray& compressedData, QString& outputString)
            {
                QByteArray decompressedData = qUncompress(compressedData);
                if (decompressedData.size() == 0)
                {
                    return false; // Corrupt data 
                }
                outputString = QString::fromUtf8(decompressedData);
                return true;
            }
        }
    }
}