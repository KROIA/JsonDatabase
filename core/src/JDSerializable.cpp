#include "JDSerializable.h"

namespace JsonDatabase
{
bool JDSerializable::getJsonValue(const simdjson::dom::object &obj, simdjson::dom::object &value, const std::string& key)
{
    simdjson::simdjson_result<simdjson::dom::element> element = obj[key];
    if (element.error() != simdjson::SUCCESS)
        return false; // Key does not exist

    if (!element.is_object())
        return false; // Element is not a object

    value = element.get_object();
    return true;
}
/*bool JDSerializable::getJsonValue(const simdjson::dom::object& obj, QVariant& value, const std::string& key)
{
    simdjson::simdjson_result<simdjson::dom::element> element = obj[key];
    if (element.error() != simdjson::SUCCESS)
        return false; // Key does not exist

    if(obj.contains(key))
    {
        QJsonValue val = obj[key];
        value = val.toVariant();
        return true;
    }
    return false;
}*/
bool JDSerializable::getJsonValue(const simdjson::dom::object &obj, QString &value, const std::string& key)
{
    simdjson::simdjson_result<simdjson::dom::element> element = obj[key];
    if (element.error() != simdjson::SUCCESS)
        return false; // Key does not exist

    if (!element.is_string())
        return false; // Element is not a string

    value = QString(element.get_c_str());
    return true;
}
bool JDSerializable::getJsonValue(const simdjson::dom::object &obj, std::string &value, const std::string& key)
{
    simdjson::simdjson_result<simdjson::dom::element> element = obj[key];
    if (element.error() != simdjson::SUCCESS)
        return false; // Key does not exist

    if (!element.is_string())
        return false; // Element is not a string

    value = element.get_c_str();
    return true;
}
bool JDSerializable::getJsonValue(const simdjson::dom::object &obj, int &value, const std::string &key)
{
    simdjson::simdjson_result<simdjson::dom::element> element = obj[key];
    if (element.error() != simdjson::SUCCESS)
        return false; // Key does not exist

    if (!element.is_int64())
        return false; // Element is not a number

    value = element.get_int64();
    return true;
}
bool JDSerializable::getJsonValue(const simdjson::dom::object &obj, double &value, const std::string& key)
{
    simdjson::simdjson_result<simdjson::dom::element> element = obj[key];
    if (element.error() != simdjson::SUCCESS)
        return false; // Key does not exist

    if (!element.is_double())
        return false; // Element is not a number

    value = element.get_double();
    return true;
}
bool JDSerializable::getJsonValue(const simdjson::dom::object &obj, float &value, const std::string& key)
{
    simdjson::simdjson_result<simdjson::dom::element> element = obj[key];
    if (element.error() != simdjson::SUCCESS)
        return false; // Key does not exist

    if (!element.is_double())
        return false; // Element is not a number

    value = element.get_double();
    return true;
}
bool JDSerializable::getJsonValue(const simdjson::dom::object &obj, bool &value, const std::string& key)
{
    simdjson::simdjson_result<simdjson::dom::element> element = obj[key];
    if (element.error() != simdjson::SUCCESS)
        return false; // Key does not exist

    if (!element.is_bool())
        return false; // Element is not a number

    value = element.get_bool();
    return true;
}
}
