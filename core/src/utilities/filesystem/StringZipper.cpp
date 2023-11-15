#include "utilities/filesystem/StringZipper.h"
#include <QtEndian>

namespace JsonDatabase
{
    namespace Internal
    {
        namespace StringZipper
        {
            void compressString(const QString& inputString, QByteArray& compressedData)
            {
                QByteArray utf8Data = inputString.toUtf8();
                QByteArray compressed = qCompress(utf8Data, -1);

                // Prepend a four-byte header with the uncompressed data size
                compressedData.resize(4);
                qint32 dataSize = utf8Data.size();
                qToBigEndian(dataSize, reinterpret_cast<uchar*>(compressedData.data()));

                compressedData.append(compressed);
            }
            bool decompressString(QByteArray compressedData, QString& outputString)
            {
                // Extract the expected uncompressed data size from the first 4 bytes
                qint32 expectedSize = qFromBigEndian<qint32>(
                    reinterpret_cast<const uchar*>(compressedData.constData()));

                // Remove the header from the compressed data
                compressedData.remove(0, 4);
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