#pragma once
#include "JD_base.h"
#include <type_traits>
#include <QDebug>

#if JD_ACTIVE_JSON == JD_JSON_QT
#include <QJsonObject>

#elif JD_ACTIVE_JSON == JD_JSON_GLAZE
#include "glaze/glaze.hpp"

namespace JsonDatabase
{
	using JsonValue = glz::json_t;
	using JsonObject = glz::json_t::object_t;
	using JsonArray = glz::json_t::array_t;

	std::ostream& operator<<(std::ostream& os, const JsonValue& json);
	std::ostream& operator<<(std::ostream& os, const JsonObject& json);
	std::ostream& operator<<(std::ostream& os, const JsonArray& json);
	QDebug operator<<(QDebug debug, const JsonValue& json);
	QDebug operator<<(QDebug debug, const JsonObject& json);
	QDebug operator<<(QDebug debug, const JsonArray& json);
}

#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
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


	using JsonArray = JsonArrayType< JsonValue>;
	using JsonObject = std::map<std::string, JsonValue>;

	/*
	class JsonObject {
	public:
		// Alias for the underlying map type
		using MapType = std::map<std::string, std::shared_ptr<JsonValue>>;

		// Iterator types
		using iterator = typename MapType::iterator;
		using const_iterator = typename MapType::const_iterator;

		bool operator==(const JsonObject& other) const
		{
			return m_map == other.m_map;
		}
		bool operator!=(const JsonObject& other) const
		{
			return m_map != other.m_map;
		}

		// Methods similar to std::map
		std::pair<iterator, bool> insert(const std::pair<std::string, std::shared_ptr<JsonValue>>& value) {
			return m_map.insert(value);
		}

		size_t erase(const std::string& key) {
			return m_map.erase(key);
		}



		std::shared_ptr<JsonValue> operator[](const std::string& key) {
			if (m_map.find(key) == m_map.end())
			{
				std::shared_ptr<JsonValue> newVal = std::make_shared<JsonValue>();
				m_map.emplace(key, newVal);
				return newVal;
			}
			return m_map[key];
		}

		size_t size() const {
			return m_map.size();
		}

		iterator begin() {
			return m_map.begin();
		}

		iterator end() {
			return m_map.end();
		}

		const_iterator begin() const {
			return m_map.begin();
		}

		const_iterator end() const {
			return m_map.end();
		}

		// Emplace function to add elements in place
		template <typename... Args>
		std::pair<iterator, bool> emplace(Args&&... args) {
			return m_map.emplace(std::forward<Args>(args)...);
		}

		// .at() function to retrieve a reference to the mapped value
		std::shared_ptr<JsonValue>& at(const std::string& key) {
			const auto &it = m_map.find(key);

			if (it == m_map.end())
			{
				static std::shared_ptr<JsonValue> nullVal;
				nullVal.reset();
				return nullVal;
			}
			
			return it->second;
		}

		// .at() function to retrieve a reference to the mapped value
		const std::shared_ptr<JsonValue>& at(const std::string& key) const {
			const auto& it = m_map.find(key);

			if (it == m_map.end())
			{
				static std::shared_ptr<JsonValue> nullVal;
				nullVal.reset();
				return nullVal;
			}

			return it->second;
		}

		// Check if the specified key exists in the map
		bool contains(const std::string& key) const {
			return (m_map.find(key) != m_map.end());
		}

		// Find an element with the given key
		iterator find(const std::string& key) {
			return m_map.find(key);
		}

		const_iterator find(const std::string& key) const {
			return m_map.find(key);
		}

		friend std::ostream& operator<<(std::ostream& os, const JsonObject& json)
		{
			os << "not implemented: " << __FILE__ << " : "<< __LINE__;
			//os << json.toString();
			return os;
		}

		// Overloading << operator for qDebug()
		friend QDebug operator<<(QDebug debug, const JsonObject& json)
		{
			debug << "not implemented: " << __FILE__ << " : " << __LINE__;;
			//QDebugStateSaver saver(debug);
			//debug << json.toString().c_str();
			return debug;
		}

	private:
		MapType m_map;
	};
	*/
	class JSONDATABASE_EXPORT JsonValue
	{
		friend class JsonSerializer;
		friend class JsonDeserializer;
		public:

		using JsonVariantType = std::variant<std::monostate, std::string, int, double, bool, std::shared_ptr<JsonArray>, std::shared_ptr<JsonObject>>;
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
		JsonValue(std::string&& value) noexcept;
		JsonValue(const char* value);
		JsonValue(const int& value);
		JsonValue(const double& value);
		JsonValue(const bool& value);
		JsonValue(const JsonArray& value);
		JsonValue(JsonArray&& value);
		JsonValue(const std::shared_ptr<JsonArray> &valuePtr);
		JsonValue(std::shared_ptr<JsonArray> &&valuePtr) noexcept;
		JsonValue(const JsonObject& value);
		JsonValue(JsonObject&& value) noexcept;
		JsonValue(const std::shared_ptr<JsonObject>& valuePtr);
		JsonValue(std::shared_ptr<JsonObject>&& valuePtr) noexcept;
		
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


		bool operator==(const JsonValue& other) const;
		bool operator!=(const JsonValue& other) const;
		// bool operator<(const JsonValue& other) const;
		// bool operator>(const JsonValue& other) const;
		// bool operator<=(const JsonValue& other) const;
		// bool operator>=(const JsonValue& other) const;

		/*template <class T>
		bool work(T obj) const
		{
			return normalWork(obj);
		}
		template <class T = ObjectA>
		bool work(T obj) const
		{
			return objectA_work(obj);
		}*/


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

		/*template <class T>
		bool holds() const noexcept
		{
			return std::holds_alternative<T>(m_value);
		}
		template <class T = JsonArray>
		bool holds() const noexcept
		{
			return std::holds_alternative<std::shared_ptr<JsonArray>>(m_value);
		}*/

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
		//template <class T = JsonObject>
		//bool holds() const noexcept
		//{
		//	return std::holds_alternative<std::shared_ptr<JsonObject>>(m_value);
		//}

		// Type getType() const;
		// bool isNull() const;
		// bool isString() const;
		// bool isNumber() const;
		// bool isInt() const;
		// bool isDouble() const;
		// bool isBool() const;
		// bool isArray() const;
		// bool isObject() const;

		//bool contains(const std::string& key) const; // returns true if this is an object and contains the key
		//JsonValue& operator[](const std::string& key); // returns the value of the key if this is an object

		/*template <class T>
		T& get()
		{
			return std::get<T>(m_value);
		}
		template <>
		JsonArray& get()
		{
			return *std::get<std::shared_ptr<JsonArray>>(m_value).get();
		}
		template <>
		JsonObject& get()
		{
			return *std::get<std::shared_ptr<JsonObject>>(m_value).get();
		}
		*/
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




		/*template <class T>
		const T& get() const
		{
			return std::get<T>(m_value);
		}
		template <>
		const JsonArray& get() const
		{
			return *std::get<std::shared_ptr<JsonArray>>(m_value).get();
		}
		template <>
		const JsonObject& get() const
		{
			return *std::get<std::shared_ptr<JsonObject>>(m_value).get();
		}*/

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



		/*
		template <class T>
		T* get_if() noexcept
		{
			return std::get_if<T>(&m_value);
		}
		template <>
		JsonArray* get_if() noexcept
		{
			std::shared_ptr<JsonArray>* ptr = std::get_if<std::shared_ptr<JsonArray>>(&m_value);
			if (ptr)
				return ptr->get();
			return nullptr;
		}
		template <>
		JsonObject* get_if() noexcept
		{
			std::shared_ptr<JsonObject>* ptr = std::get_if<std::shared_ptr<JsonObject>>(&m_value);
			if (ptr)
				return ptr->get();
			return nullptr;
		}*/

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



		/*template <class T>
		const T* get_if() const noexcept
		{
			return std::get_if<T>(&m_value);
		}
		template <>
		const JsonArray* get_if() noexcept
		{
			const std::shared_ptr<JsonArray>* ptr = std::get_if<std::shared_ptr<JsonArray>>(&m_value);
			if (ptr)
				return ptr->get();
			return nullptr;
		}
		template <>
		const JsonObject* get_if() noexcept
		{
			const std::shared_ptr<JsonObject> *ptr = std::get_if<std::shared_ptr<JsonObject>>(&m_value);
			if(ptr)
				return ptr->get();
			return nullptr;
		}
		*/
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




		/*
		JsonValue& operator[](std::integral auto&& index) 
		{ 
			std::shared_ptr<JsonArray>& array = std::get<std::shared_ptr<JsonArray>>(m_value);
			return (*array.get())[index];
		}

		const JsonValue& operator[](std::integral auto&& index) const 
		{ 
			const std::shared_ptr<JsonArray>& array = std::get<std::shared_ptr<JsonArray>>(m_value);
			return (*array.get())[index];
		}

		JsonValue& operator[](std::convertible_to<std::string_view> auto&& key)
		{
			//[] operator for maps does not support heterogeneous lookups yet
			if (holds<std::monostate>())
				m_value = std::make_shared<JsonObject>();
			std::shared_ptr<JsonObject>& object = std::get<std::shared_ptr<JsonObject>>(m_value);
			auto iter = object->find(key);
			if (iter == object->end()) {
				iter = object->insert(std::make_pair(std::string(key), JsonValue{})).first;
			}
			return *iter->second->get();
		}

		const JsonValue& operator[](std::convertible_to<std::string_view> auto&& key) const
		{
			//[] operator for maps does not support heterogeneous lookups yet
			std::shared_ptr<JsonObject>& object = std::get<std::shared_ptr<JsonObject>>(m_value);
			auto iter = object->find(key);
			if (iter == object->end()) {
				static const JsonValue null{};
				return null;
			}
			return *iter->second->get();
		}*/



		//std::string &getString();
		//int& getInt();
		//double& getDouble();
		//bool& getBool();
		//JsonArray& getArray();
		//JsonObject& getObject();
		//
		//const std::string& getString() const;
		//const int& getInt() const;
		//const double& getDouble() const;
		//const bool& getBool() const;
		//const JsonArray& getArray() const;
		//const JsonObject& getObject() const;

		// bool extractString(std::string& valueOut) const;
		// //bool getString(JsonValue& objOut, const std::string &key) const;
		// bool extractInt(int& valueOut) const;
		// bool extractDouble(double& valueOut) const;
		// bool extractBool(bool& valueOut) const;
		// bool extractArray(JsonArray& valueOut) const;
		// bool extractObject(JsonObject& valueOut) const;
		// 
		// bool extractString(std::string& valueOut, const std::string& key) const;
		// bool extractInt(int& valueOut, const std::string& key) const;
		// bool extractDouble(double& valueOut, const std::string& key) const;
		// bool extractBool(bool& valueOut, const std::string& key) const;
		// bool extractArray(JsonArray& valueOut, const std::string& key) const;
		// bool extractObject(JsonObject& valueOut, const std::string& key) const;

		// std::string& getString(const std::string& key);
		// int &getInt(const std::string& key);
		// double& getDouble(const std::string& key);
		// bool& getBool(const std::string& key);
		// JsonArray& getArray(const std::string& key);
		// JsonObject& getObject(const std::string& key);
		// 
		// const std::string& getString(const std::string& key) const;
		// const int& getInt(const std::string& key) const;
		// const double& getDouble(const std::string& key) const;
		// const bool& getBool(const std::string& key) const;
		// const JsonArray& getArray(const std::string& key) const;
		// const JsonObject& getObject(const std::string& key) const;
		// 
		// std::string* getStringPtr(const std::string& key);
		// int* getIntPtr(const std::string& key);
		// double* getDoublePtr(const std::string& key);
		// bool* getBoolPtr(const std::string& key);
		// JsonArray* getArrayPtr(const std::string& key);
		// JsonObject* getObjectPtr(const std::string& key);
		// 
		// const std::string* getStringPtr(const std::string& key) const;
		// const int* getIntPtr(const std::string& key) const;
		// const double* getDoublePtr(const std::string& key) const;
		// const bool* getBoolPtr(const std::string& key) const;
		// const JsonArray* getArrayPtr(const std::string& key) const;
		// const JsonObject* getObjectPtr(const std::string& key) const;

		JsonVariantType* operator->() noexcept { return &m_value; }

		JsonVariantType& operator*() noexcept { return m_value; }
		const JsonVariantType& operator*() const noexcept { return m_value; }

		std::string toString() const;
		std::string serialize() const;

		friend std::ostream& operator<<(std::ostream& os, const JsonValue& json);
		friend QDebug operator<<(QDebug debug, const JsonValue& json);
	private:
		
		//Type m_type;
		JsonVariantType m_value;
		//JsonObject* m_objElement; // is active, if this is an JsonObject
		
	};
}
#endif
