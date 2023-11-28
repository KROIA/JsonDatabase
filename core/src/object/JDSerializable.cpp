#include "object/JDSerializable.h"

namespace JsonDatabase
{
#ifdef JD_USE_QJSON
bool JDSerializable::getJsonValue(const QJsonObject &obj, QJsonObject &value, const QString &key)
#else
    bool JDSerializable::getJsonValue(const JsonObject& obj, JsonObject& value, const std::string& key)
#endif
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_5);
    if(obj.contains(key))
    {
        JD_GENERAL_PROFILING_BLOCK("contains key",JD_COLOR_STAGE_6);
#ifdef JD_USE_QJSON
        QJsonValue val = obj[key];
#else
        const JsonValue &val = obj.find(key)->second;
#endif
        if(val.isObject())
        {
            JD_GENERAL_PROFILING_BLOCK("is object", JD_COLOR_STAGE_7);
#ifdef JD_USE_QJSON
            value = val.toObject();
#else
            value = val.getObject();
#endif
            return true;
        }
        else
            return false;
    }
    return false;
}
#ifdef JD_USE_QJSON
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
#endif
#ifdef JD_USE_QJSON
bool JDSerializable::getJsonValue(const QJsonObject &obj, QString &value, const QString &key)
#else
bool JDSerializable::getJsonValue(const JsonObject& obj, QString& value, const std::string& key)
#endif
{
    if(obj.contains(key))
    {
#ifdef JD_USE_QJSON
        value = obj[key].toString();
#else
        value = QString::fromStdString(obj.find(key)->second.getString());
#endif
        return true;
    }
    return false;
}
#ifdef JD_USE_QJSON
bool JDSerializable::getJsonValue(const QJsonObject &obj, std::string &value, const QString &key)
#else
bool JDSerializable::getJsonValue(const JsonObject& obj, std::string& value, const std::string& key)
#endif
{
    if(obj.contains(key))
    {
#ifdef JD_USE_QJSON
        value = obj[key].toString().toStdString();
#else
        value = obj.find(key)->second.getString();
#endif
        return true;
    }
    return false;
}

#ifdef JD_USE_QJSON
bool JDSerializable::getJsonValue(const QJsonObject &obj, int &value, const QString &key)
#else
bool JDSerializable::getJsonValue(const JsonObject& obj, int& value, const std::string& key)
#endif
{
    if(obj.contains(key))
    {
#ifdef JD_USE_QJSON
        value = obj[key].toInt(value);
#else
        value = obj.find(key)->second.getInt();
#endif
        return true;
    }
    return false;
}
/*
#ifdef JD_USE_QJSON
bool JDSerializable::getJsonValue(const QJsonObject& obj, JDObjectID::IDType& value, const QString& key)
#else
bool JDSerializable::getJsonValue(const JsonObject& obj, JDObjectID::IDType& value, const std::string& key)
#endif
{
    if (obj.contains(key))
    {
#ifdef JD_USE_QJSON
        value = obj[key].toInt(value);
#else
        value = obj.find(key)->second.getInt();
#endif
        return true;
    }
    return false;
}
*/
#ifdef JD_USE_QJSON
bool JDSerializable::getJsonValue(const QJsonObject &obj, double &value, const QString &key)
#else
bool JDSerializable::getJsonValue(const JsonObject& obj, double& value, const std::string& key)
#endif
{
    if(obj.contains(key))
    {
#ifdef JD_USE_QJSON
        value = obj[key].toDouble(value);
#else
        value = obj.find(key)->second.getDouble();
#endif
        return true;
    }
    return false;
}

#ifdef JD_USE_QJSON
bool JDSerializable::getJsonValue(const QJsonObject &obj, float &value, const QString &key)
#else
bool JDSerializable::getJsonValue(const JsonObject& obj, float& value, const std::string& key)
#endif
{
    if(obj.contains(key))
    {
#ifdef JD_USE_QJSON
        value = obj[key].toDouble(value);
#else
        value = obj.find(key)->second.getDouble();
#endif
        return true;
    }
    return false;
}
#ifdef JD_USE_QJSON
bool JDSerializable::getJsonValue(const QJsonObject &obj, bool &value, const QString &key)
#else
bool JDSerializable::getJsonValue(const JsonObject& obj, bool& value, const std::string& key)
#endif
{
    if(obj.contains(key))
    {
#ifdef JD_USE_QJSON
        value = obj[key].toBool(value);
#else
        value = obj.find(key)->second.getBool();
#endif
        return true;
    }
    return false;
}

}
