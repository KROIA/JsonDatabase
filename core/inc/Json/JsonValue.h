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

		using JsonVariantType = std::variant<std::monostate, std::string, int, double, bool, JsonArray, JsonObject>;
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

		bool contains(const std::string& key) const; // returns true if this is an object and contains the key
		JsonValue& operator[](const std::string& key); // returns the value of the key if this is an object


		std::string &getString();
		int& getInt();
		double& getDouble();
		bool& getBool();
		JsonArray& getArray();
		JsonObject& getObject();

		const std::string& getString() const;
		const int& getInt() const;
		const double& getDouble() const;
		const bool& getBool() const;
		const JsonArray& getArray() const;
		const JsonObject& getObject() const;

		bool extractString(std::string& valueOut) const;
		//bool getString(JsonValue& objOut, const std::string &key) const;
		bool extractInt(int& valueOut) const;
		bool extractDouble(double& valueOut) const;
		bool extractBool(bool& valueOut) const;
		bool extractArray(JsonArray& valueOut) const;
		bool extractObject(JsonObject& valueOut) const;

		bool extractString(std::string& valueOut, const std::string& key) const;
		bool extractInt(int& valueOut, const std::string& key) const;
		bool extractDouble(double& valueOut, const std::string& key) const;
		bool extractBool(bool& valueOut, const std::string& key) const;
		bool extractArray(JsonArray& valueOut, const std::string& key) const;
		bool extractObject(JsonObject& valueOut, const std::string& key) const;

		std::string& getString(const std::string& key);
		int &getInt(const std::string& key);
		double& getDouble(const std::string& key);
		bool& getBool(const std::string& key);
		JsonArray& getArray(const std::string& key);
		JsonObject& getObject(const std::string& key);

		const std::string& getString(const std::string& key) const;
		const int& getInt(const std::string& key) const;
		const double& getDouble(const std::string& key) const;
		const bool& getBool(const std::string& key) const;
		const JsonArray& getArray(const std::string& key) const;
		const JsonObject& getObject(const std::string& key) const;

		std::string* getStringPtr(const std::string& key);
		int* getIntPtr(const std::string& key);
		double* getDoublePtr(const std::string& key);
		bool* getBoolPtr(const std::string& key);
		JsonArray* getArrayPtr(const std::string& key);
		JsonObject* getObjectPtr(const std::string& key);

		const std::string* getStringPtr(const std::string& key) const;
		const int* getIntPtr(const std::string& key) const;
		const double* getDoublePtr(const std::string& key) const;
		const bool* getBoolPtr(const std::string& key) const;
		const JsonArray* getArrayPtr(const std::string& key) const;
		const JsonObject* getObjectPtr(const std::string& key) const;

		JsonVariantType &getVariant();
		const JsonVariantType &getConstVariant() const;

		std::string toString() const;
		std::string serialize() const;

		friend std::ostream& operator<<(std::ostream& os, const JsonValue& json);
		friend QDebug operator<<(QDebug debug, const JsonValue& json);
	private:
		
		Type m_type;
		JsonVariantType m_value;
		JsonObject* m_objElement; // is active, if this is an JsonObject
		
	};
}