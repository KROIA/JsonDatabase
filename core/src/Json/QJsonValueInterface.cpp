#include "Json/QJsonValueInterface.h"

#if JD_ACTIVE_JSON == JD_JSON_QT
namespace JsonDatabase
{
	JsonArray::JsonArray()
	{	}
	JsonArray::JsonArray(const JsonArray& other)
		: m_array(other.m_array)
	{	}
	JsonArray::JsonArray(JsonArray&& other) noexcept
		: m_array(std::move(other.m_array))
	{	}
	JsonArray::JsonArray(const QJsonArray& other)
		: m_array(other)
	{	}
	JsonArray::JsonArray(QJsonArray&& other) noexcept
		: m_array(std::move(other))
	{	}

	JsonArray& JsonArray::operator=(const JsonArray& other)
	{
		m_array = other.m_array; 
		return *this;
	}
	JsonArray& JsonArray::operator=(JsonArray&& other) noexcept
	{
		m_array = std::move(other.m_array);
		return *this;
	}
	JsonArray& JsonArray::operator=(const QJsonArray& other)
	{
		m_array = other;
		return *this;
	}
	JsonArray& JsonArray::operator=(QJsonArray&& other) noexcept
	{
		m_array = std::move(other);
		return *this;
	}

	bool JsonArray::operator==(const JsonArray& other) const
	{
		return m_array == other.m_array;
	}
	bool JsonArray::operator==(const QJsonArray& other) const
	{
		return m_array == other;
	}
	bool JsonArray::operator!=(const JsonArray& other) const
	{
		return m_array != other.m_array;
	}
	bool JsonArray::operator!=(const QJsonArray& other) const
	{
		return m_array != other;
	}




	JsonObject::JsonObject()
	{	}
	JsonObject::JsonObject(const JsonObject& other)
		: m_obj(other.m_obj)
	{	}
	JsonObject::JsonObject(JsonObject&& other) noexcept
		: m_obj(std::move(other.m_obj))
	{	}
	JsonObject::JsonObject(const QJsonObject& other) 
		: m_obj(other)
	{	}
	JsonObject::JsonObject(QJsonObject&& other) noexcept
		: m_obj(std::move(other))
	{	}

	JsonObject& JsonObject::operator=(const JsonObject& other)
	{
		m_obj = other.m_obj;
		return *this;
	}
	JsonObject& JsonObject::operator=(JsonObject&& other) noexcept
	{
		m_obj = std::move(other.m_obj);
		return *this;
	}
	JsonObject& JsonObject::operator=(const QJsonObject& other)
	{
		m_obj = other;
		return *this;
	}
	JsonObject& JsonObject::operator=(QJsonObject&& other) noexcept
	{
		m_obj = std::move(other);
		return *this;
	}

	bool JsonObject::operator==(const JsonObject& other) const
	{
		return m_obj == other.m_obj;
	}
	bool JsonObject::operator==(const QJsonObject& other) const
	{
		return m_obj == other;
	}
	bool JsonObject::operator!=(const JsonObject& other) const
	{
		return m_obj != other.m_obj;
	}
	bool JsonObject::operator!=(const QJsonObject& other) const
	{
		return m_obj != other;
	}





	JsonValue::JsonValue()
	{	}
	JsonValue::JsonValue(const JsonValue& other)
		: m_value(other.m_value)
	{	}	
	JsonValue::JsonValue(JsonValue&& other) noexcept
		: m_value(std::move(other.m_value))
	{	}
	JsonValue::JsonValue(const std::string& value)
		: m_value(value.c_str())
	{	}
	JsonValue::JsonValue(std::string&& value) noexcept
		: m_value(value.c_str())
	{	}
	JsonValue::JsonValue(const char* value)
		: m_value(value)
	{	}
	JsonValue::JsonValue(int value)
		: m_value(value)
	{	}
	JsonValue::JsonValue(double value)
		: m_value(value)
	{	}
	JsonValue::JsonValue(bool value)
		: m_value(value)
	{	}
	JsonValue::JsonValue(const JsonArray& value)
		: m_value(value.m_array)
	{	}
	JsonValue::JsonValue(JsonArray&& value) noexcept
		: m_value(std::move(value.m_array))
	{	}
	JsonValue::JsonValue(const std::shared_ptr<JsonArray>& valuePtr)
		: m_value(valuePtr->m_array)
	{	}
	JsonValue::JsonValue(std::shared_ptr<JsonArray>&& valuePtr) noexcept
		: m_value(std::move(valuePtr->m_array))
	{	}
	JsonValue::JsonValue(const JsonObject& value)
		: m_value(value.m_obj)
	{	}
	JsonValue::JsonValue(JsonObject&& value) noexcept
		: m_value(std::move(value.m_obj))
	{	}
	JsonValue::JsonValue(const std::shared_ptr<JsonObject>& valuePtr)
		: m_value(valuePtr->m_obj)
	{	}
	JsonValue::JsonValue(std::shared_ptr<JsonObject>&& valuePtr) noexcept
		: m_value(std::move(valuePtr->m_obj))
	{	}
	
	JsonValue::JsonValue(const QJsonValue& value)
		: m_value(value)
	{	}
	JsonValue::JsonValue(QJsonValue&& value) noexcept
		: m_value(std::move(value))
	{	}

	JsonValue::~JsonValue()
	{	}





	JsonValue& JsonValue::operator=(const JsonValue& other)
	{
		m_value = other.m_value;
		return *this;
	}
	JsonValue& JsonValue::operator=(JsonValue&& other) noexcept
	{
		m_value = std::move(other.m_value);
		return *this;
	}
	JsonValue& JsonValue::operator=(const std::string& value)
	{
		m_value = value.c_str();
		return *this;
	}
	JsonValue& JsonValue::operator=(std::string&& value) noexcept
	{
		m_value = std::move(value.c_str());
		return *this;
	}
	JsonValue& JsonValue::operator=(const char* value)
	{
		m_value = value;
		return *this;
	}
	JsonValue& JsonValue::operator=(const int& value)
	{
		m_value = value;
		return *this;
	}
	JsonValue& JsonValue::operator=(const double& value)
	{
		m_value = value;
		return *this;
	}
	JsonValue& JsonValue::operator=(const bool& value)
	{
		m_value = value;
		return *this;
	}
	JsonValue& JsonValue::operator=(const JsonArray& value)
	{
		m_value = value.m_array;
		return *this;
	}
	JsonValue& JsonValue::operator=(const JsonObject& value)
	{
		m_value = value.m_obj;
		return *this;
	}
	JsonValue& JsonValue::operator=(const std::shared_ptr<JsonArray>& value)
	{
		m_value = value->m_array;
		return *this;
	}
	JsonValue& JsonValue::operator=(const std::shared_ptr<JsonObject>& value)
	{
		m_value = value->m_obj;
		return *this;
	}
	JsonValue& JsonValue::operator=(JsonArray&& value) noexcept
	{
		m_value = std::move(value.m_array);
		return *this;
	}
	JsonValue& JsonValue::operator=(JsonObject&& value) noexcept
	{
		m_value = std::move(value.m_obj);
		return *this;
	}
	JsonValue& JsonValue::operator=(std::shared_ptr<JsonArray>&& value) noexcept
	{
		m_value = std::move(value->m_array);
		return *this;
	}
	JsonValue& JsonValue::operator=(std::shared_ptr<JsonObject>&& value) noexcept
	{
		m_value = std::move(value->m_obj);
		return *this;
	}
	JsonValue& JsonValue::operator=(const QJsonValue& value)
	{
		m_value = value;
		return *this;
	}
	JsonValue& JsonValue::operator=(QJsonValue&& value) noexcept
	{
		m_value = std::move(value);
		return *this;
	}


	bool JsonValue::operator==(const JsonValue& other) const
	{
		return m_value == other.m_value;
	}
	bool JsonValue::operator==(const QJsonValue& other) const
	{
		return m_value == other;
	}
	bool JsonValue::operator!=(const JsonValue& other) const
	{
		return m_value != other.m_value;
	}
	bool JsonValue::operator!=(const QJsonValue& other) const
	{
		return m_value != other;
	}
}
#endif