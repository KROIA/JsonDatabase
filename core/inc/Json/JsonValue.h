#pragma once
#include "JD_base.h"
#include <variant>
#include <string>
#include <vector>
#include <unordered_map>

namespace JsonDatabase
{
	class JsonValue;
	
	template<class T>
	using JsonArrayType = std::vector<T>;

	template<class K, class V>
	using JsonMapType = std::unordered_map<K, V>;

	

	using JsonArray = JsonArrayType<JsonValue>;
	using JsonObject = JsonMapType<std::string, JsonValue>;

	class JSONDATABASE_EXPORT JsonValue
	{
		friend class JsonSerializer;
		friend class JsonDeserializer;
		public:
			enum class Type 
			{
				Null,
				String,
				Int,
				Double,
				Bool,
				Array,
				Object
			};

		JsonValue();
		JsonValue(const JsonValue& other);
		JsonValue(JsonValue&& other) noexcept;
		JsonValue(const std::string& value);
		JsonValue(const char* value);
		JsonValue(const int& value);
		JsonValue(const double& value);
		JsonValue(const bool& value);
		JsonValue(const JsonArray& value);
		JsonValue(const JsonObject& value);
		JsonValue(JsonObject&& value) noexcept;
		~JsonValue();

		JsonValue& operator=(const JsonValue& other);
		JsonValue& operator=(JsonValue&& other) noexcept;
		JsonValue& operator=(const std::string& value);
		JsonValue& operator=(const char* value);
		JsonValue& operator=(const int& value);
		JsonValue& operator=(const double& value);
		JsonValue& operator=(const bool& value);
		JsonValue& operator=(const JsonArray& value);
		JsonValue& operator=(const JsonObject& value);
		JsonValue& operator=(JsonObject&& value) noexcept;

		bool operator==(const JsonValue& other) const;
		bool operator!=(const JsonValue& other) const;
		// bool operator<(const JsonValue& other) const;
		// bool operator>(const JsonValue& other) const;
		// bool operator<=(const JsonValue& other) const;
		// bool operator>=(const JsonValue& other) const;

		Type getType() const;
		bool isNull() const;
		bool isString() const;
		bool isNumber() const;
		bool isInt() const;
		bool isDouble() const;
		bool isBool() const;
		bool isArray() const;
		bool isObject() const;

		std::string toString() const;
		std::string serialize() const;

		friend std::ostream& operator<<(std::ostream& os, const JsonValue& json);
		friend QDebug operator<<(QDebug debug, const JsonValue& json);
	private:
		
		Type m_type;
		std::variant<std::monostate, std::string, int, double, bool, JsonArray, JsonObject> m_value;

		
	};
}