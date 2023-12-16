#pragma once
#include "JD_base.h"

#if JD_ACTIVE_JSON == JD_JSON_QT
#include <QJsonObject>
#include <QJsonArray>


namespace JsonDatabase
{
	class JsonValue;

	class JSONDATABASE_EXPORT JsonArray
	{
		friend JsonValue;
	public:
		JsonArray();
		JsonArray(const JsonArray& other);
		JsonArray(JsonArray&& other) noexcept;
		JsonArray(const QJsonArray& other);
		JsonArray(QJsonArray&& other) noexcept;

		JsonArray& operator=(const JsonArray& other);
		JsonArray& operator=(JsonArray&& other) noexcept;
		JsonArray& operator=(const QJsonArray& other);
		JsonArray& operator=(QJsonArray&& other) noexcept;

		bool operator==(const JsonArray& other) const;
		bool operator==(const QJsonArray& other) const;
		bool operator!=(const JsonArray& other) const;
		bool operator!=(const QJsonArray& other) const;


	private:
		QJsonArray m_array;
	};

	class JSONDATABASE_EXPORT JsonObject
	{
		friend JsonValue;
	public:
		JsonObject();
		JsonObject(const JsonObject& other);
		JsonObject(JsonObject&& other) noexcept;
		JsonObject(const QJsonObject& other);
		JsonObject(QJsonObject && other) noexcept;

		JsonObject& operator=(const JsonObject& other);
		JsonObject& operator=(JsonObject&& other) noexcept;
		JsonObject& operator=(const QJsonObject& other);
		JsonObject& operator=(QJsonObject&& other) noexcept;

		bool operator==(const JsonObject& other) const;
		bool operator==(const QJsonObject& other) const;
		bool operator!=(const JsonObject& other) const;
		bool operator!=(const QJsonObject& other) const;

	private:
		QJsonObject m_obj;
	};


	class JSONDATABASE_EXPORT JsonValue
	{
		friend class JsonSerializer;
		friend class JsonDeserializer;
	public:
		JsonValue();
		JsonValue(const JsonValue& other);
		JsonValue(JsonValue&& other) noexcept;
		JsonValue(const std::string& value);
		JsonValue(std::string&& value) noexcept;
		JsonValue(const char* value);
		explicit JsonValue(int value);
		explicit JsonValue(double value);
		explicit JsonValue(bool value);
		JsonValue(const JsonArray& value);
		JsonValue(JsonArray&& value) noexcept;
		JsonValue(const std::shared_ptr<JsonArray>& valuePtr);
		JsonValue(std::shared_ptr<JsonArray>&& valuePtr) noexcept;
		JsonValue(const JsonObject& value);
		JsonValue(JsonObject&& value) noexcept;
		JsonValue(const std::shared_ptr<JsonObject>& valuePtr);
		JsonValue(std::shared_ptr<JsonObject>&& valuePtr) noexcept;

		JsonValue(const QJsonValue& value);
		JsonValue(QJsonValue&& value) noexcept;

		~JsonValue();


		JsonValue& operator=(const JsonValue& other);
		JsonValue& operator=(JsonValue&& other) noexcept;
		JsonValue& operator=(const std::string& value);
		JsonValue& operator=(std::string&& value) noexcept;
		JsonValue& operator=(const char* value);
		JsonValue& operator=(const int& value);
		JsonValue& operator=(const double& value);
		JsonValue& operator=(const bool& value);
		JsonValue& operator=(const JsonArray& value);
		JsonValue& operator=(const JsonObject& value);
		JsonValue& operator=(const std::shared_ptr<JsonArray>& value);
		JsonValue& operator=(const std::shared_ptr<JsonObject>& value);
		JsonValue& operator=(JsonArray&& value) noexcept;
		JsonValue& operator=(JsonObject&& value) noexcept;
		JsonValue& operator=(std::shared_ptr<JsonArray>&& value) noexcept;
		JsonValue& operator=(std::shared_ptr<JsonObject>&& value) noexcept;
		JsonValue& operator=(const QJsonValue& value);
		JsonValue& operator=(QJsonValue&& value) noexcept;


		bool operator==(const JsonValue& other) const;
		bool operator==(const QJsonValue& other) const;
		bool operator!=(const JsonValue& other) const;
		bool operator!=(const QJsonValue& other) const;


		template<typename T>
		bool holds() { return false; }
		template<> bool holds<std::string>()	{	return m_value.isString();	}
		template<> bool holds<int>()			{	return m_value.isDouble();	}
		template<> bool holds<double>()			{	return m_value.isDouble();	}
		template<> bool holds<bool>()			{	return m_value.isBool();	}
		template<> bool holds<JsonArray>()		{	return m_value.isArray();	}
		template<> bool holds<JsonObject>()		{	return m_value.isObject();	}


		template<typename T> 
		T get() { return T; }
		template<> std::string get<std::string>()	{	return m_value.toString().toStdString();	}
		template<> int get<int>()					{	return m_value.toInt();	}
		template<> double get<double>()				{	return m_value.toDouble();	}
		template<> bool get<bool>()					{	return m_value.toBool();	}
		template<> JsonArray get<JsonArray>()		{	return JsonArray(m_value.toArray());	}
		template<> JsonObject get<JsonObject>()		{	return JsonObject(m_value.toObject());	}


		std::string toString() const;
		std::string serialize() const;

		// Overloading << operator for std::cout
		friend std::ostream& operator<<(std::ostream& os, const JsonValue& json);
		friend QDebug operator<<(QDebug debug, const JsonValue& json);

	private:
		QJsonValue m_value;
	};
}

#endif