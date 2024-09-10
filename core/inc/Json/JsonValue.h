#pragma once
#include "JsonDatabase_base.h"
#include <QDebug>
#include <type_traits>

#include <variant>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>

namespace JsonDatabase
{


	class JsonValue;

	template<class T>
	using JsonArrayType = std::vector<T>;

	template<class K, class V>
	using JsonMapType = std::map<K, V>;


	using JsonArray = JsonArrayType< JsonValue>;
	using JsonObject = JsonMapType<std::string, JsonValue>;

	class JSON_DATABASE_EXPORT JsonValue
	{
		friend class JsonSerializer;
		friend class JsonDeserializer;
	public:

		using JsonVariantType = std::variant<std::monostate, std::string, long, double, bool, std::shared_ptr<JsonArray>, std::shared_ptr<JsonObject>>;
		enum class Type
		{
			Null,
			String,
			Long,
			Double,
			Bool,
			Array,
			Object
		};

		JsonValue();
		JsonValue(const JsonValue& other);
		JsonValue(JsonValue&& other) noexcept;
		JsonValue(const std::string& value);
		JsonValue(std::string&& value) noexcept;
		JsonValue(const char* value);
		JsonValue(const long& value);
		JsonValue(const double& value);
		JsonValue(const bool& value);
		JsonValue(const JsonArray& value);
		JsonValue(JsonArray&& value);
		JsonValue(const std::shared_ptr<JsonArray>& valuePtr);
		JsonValue(std::shared_ptr<JsonArray>&& valuePtr) noexcept;
		JsonValue(const JsonObject& value);
		JsonValue(JsonObject&& value) noexcept;
		JsonValue(const std::shared_ptr<JsonObject>& valuePtr);
		JsonValue(std::shared_ptr<JsonObject>&& valuePtr) noexcept;

		~JsonValue()
		{

		}

		JsonValue& operator=(const JsonValue& other);
		JsonValue& operator=(JsonValue&& other) noexcept;
		JsonValue& operator=(const std::string& value);
		JsonValue& operator=(std::string&& value) noexcept;
		JsonValue& operator=(const char* value);
		JsonValue& operator=(const long& value);
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


		bool operator==(const JsonValue& other) const;
		bool operator!=(const JsonValue& other) const;



		// Type trait to check if T is ObjectA
		template <typename T>
		struct is_SharedPtr {
			static constexpr bool value = false;
		};
		template <>
		struct is_SharedPtr<JsonArray> {
			static constexpr bool value = true;
		};
		template <>
		struct is_SharedPtr<JsonObject> {
			static constexpr bool value = true;
		};



		template <class T>
		typename std::enable_if<!is_SharedPtr<T>::value, bool>::type holds() const noexcept
		{
			return std::holds_alternative<T>(m_value);
		}

		// Overload for ObjectA
		template <class T>
		typename std::enable_if<is_SharedPtr<T>::value, bool>::type holds() const noexcept
		{
			return std::holds_alternative<std::shared_ptr<T>>(m_value);
		}

		template <class T>
		typename std::enable_if<!is_SharedPtr<T>::value, T&>::type get()
		{
			return std::get<T>(m_value);
		}
		// Overload for ObjectA
		template <class T>
		typename std::enable_if<is_SharedPtr<T>::value, T&>::type get()
		{
			return *std::get<std::shared_ptr<T>>(m_value).get();
		}




		template <class T>
		typename std::enable_if<!is_SharedPtr<T>::value, const T&>::type get() const
		{
			return std::get<T>(m_value);
		}
		// Overload for ObjectA
		template <class T>
		typename std::enable_if<is_SharedPtr<T>::value, const T&>::type get() const
		{
			return *std::get<std::shared_ptr<T>>(m_value).get();
		}


		bool isArray() const noexcept
		{
			return holds<JsonArray>();
		}
		bool isObject() const noexcept
		{
			return holds<JsonObject>();
		}


		template <class T>
		typename std::enable_if<!is_SharedPtr<T>::value, T*>::type get_if() noexcept
		{
			return std::get_if<T>(&m_value);
		}
		// Overload for ObjectA
		template <class T>
		typename std::enable_if<is_SharedPtr<T>::value, T*>::type get_if() noexcept
		{
			std::shared_ptr<T>* ptr = std::get_if<std::shared_ptr<T>>(&m_value);
			if (ptr)
				return ptr->get();
			return nullptr;
		}




		template <class T>
		typename std::enable_if<!is_SharedPtr<T>::value, const T*>::type get_if() const noexcept
		{
			return std::get_if<T>(&m_value);
		}
		// Overload for ObjectA
		template <class T>
		typename std::enable_if<is_SharedPtr<T>::value, const T*>::type get_if() const noexcept
		{
			const std::shared_ptr<T>* ptr = std::get_if<std::shared_ptr<T>>(&m_value);
			if (ptr)
				return ptr->get();
			return nullptr;
		}





		JsonVariantType* operator->() noexcept { return &m_value; }

		JsonVariantType& operator*() noexcept { return m_value; }
		const JsonVariantType& operator*() const noexcept { return m_value; }

		std::string toString() const;
		std::string serialize() const;

		friend std::ostream& operator<<(std::ostream& os, const JsonValue& json);
		friend QDebug operator<<(QDebug debug, const JsonValue& json);

		// Overloading << operator for std::cout

	private:
		JsonVariantType m_value;
	};
}


