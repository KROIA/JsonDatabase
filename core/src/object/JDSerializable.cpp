#include "object/JDSerializable.h"

namespace JsonDatabase
{
bool JDSerializable::getJsonValue(const QJsonObject &obj, QJsonObject &value, const QString &key)
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_5);
    if(obj.contains(key))
    {
        JD_GENERAL_PROFILING_BLOCK("contains key",JD_COLOR_STAGE_6);
        QJsonValue val = obj[key];
        if(val.isObject())
        {
            JD_GENERAL_PROFILING_BLOCK("is object", JD_COLOR_STAGE_7);
            value = val.toObject();
            return true;
        }
        else
            return false;
    }
    return false;
}
bool JDSerializable::getJsonValue(const QJsonObject &obj, QVariant &value, const QString &key)
{
    if(obj.contains(key))
    {
        QJsonValue val = obj[key];
        value = val.toVariant();
        return true;
    }
    return false;
}
bool JDSerializable::getJsonValue(const QJsonObject &obj, QString &value, const QString &key)
{
    if(obj.contains(key))
    {
        value = obj[key].toString();
        return true;
    }
    return false;
}
bool JDSerializable::getJsonValue(const QJsonObject &obj, std::string &value, const QString &key)
{
    if(obj.contains(key))
    {
        value = obj[key].toString().toStdString();
        return true;
    }
    return false;
}
bool JDSerializable::getJsonValue(const QJsonObject &obj, int &value, const QString &key)
{
    if(obj.contains(key))
    {
        value = obj[key].toInt(value);
        return true;
    }
    return false;
}
bool JDSerializable::getJsonValue(const QJsonObject &obj, double &value, const QString &key)
{
    if(obj.contains(key))
    {
        value = obj[key].toDouble(value);
        return true;
    }
    return false;
}
bool JDSerializable::getJsonValue(const QJsonObject &obj, float &value, const QString &key)
{
    if(obj.contains(key))
    {
        value = obj[key].toDouble(value);
        return true;
    }
    return false;
}
bool JDSerializable::getJsonValue(const QJsonObject &obj, bool &value, const QString &key)
{
    if(obj.contains(key))
    {
        value = obj[key].toBool(value);
        return true;
    }
    return false;
}
}
