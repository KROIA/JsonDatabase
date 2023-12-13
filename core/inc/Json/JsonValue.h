#pragma once
#include "JD_base.h"
#include <type_traits>

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
/*
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
		
        ~JsonValue()
        {

        }

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
		*/
            JsonValue()
                : m_value(std::monostate{})
                // , m_type(Type::Null) 
                // , m_objElement(nullptr)
            {
                // Implement default constructor logic here
            }

            // Copy constructor
            JsonValue(const JsonValue& other)
                : m_value(other.m_value)
                // , m_type(other.m_type)
                // , m_objElement(nullptr)
            {
                // Implement copy constructor logic here
                //if (m_type == Type::Object)
                //    m_objElement = &std::get<JsonObject>(m_value);
            }

            // Move constructor
            JsonValue(JsonValue&& other) noexcept
                : m_value(std::move(other.m_value))
                // , m_type(std::move(other.m_type))
                // , m_objElement(nullptr)
            {
                //if (m_type == Type::Object)
                //    m_objElement = &std::get<JsonObject>(m_value);
                // Implement move constructor logic here
            }

            // Constructor with std::string value
            JsonValue(const std::string& value)
                : m_value(value)
                //, m_type(Type::String)
                //, m_objElement(nullptr)
            {
                // Implement constructor with std::string logic here
            }
            JsonValue(std::string&& value) noexcept
                : m_value(std::move(value))
            {

            }

            // Constructor with const char* value
            JsonValue(const char* value)
                : m_value(std::string(value))
                //, m_type(Type::String)
                //, m_objElement(nullptr)
            {
                // Implement constructor with const char* logic here
            }

            // Constructor with int value
            JsonValue(const int& value)
                : m_value(value)
                //, m_type(Type::Int)
                //, m_objElement(nullptr)
            {
                // Implement constructor with int logic here
            }

            // Constructor with double value
            JsonValue(const double& value)
                : m_value(value)
                //, m_type(Type::Double)
                //, m_objElement(nullptr)
            {
                // Implement constructor with double logic here
            }

            // Constructor with bool value
            JsonValue(const bool& value)
                : m_value(value)
                //, m_type(Type::Bool)
                //, m_objElement(nullptr)
            {
                // Implement constructor with bool logic here
            }

            // Constructor with std::vector<JsonValue> value
            JsonValue(const JsonArray& value)
                : m_value(std::make_shared<JsonArray>(value))
                //, m_type(Type::Array)
                //, m_objElement(nullptr)
            {
                // Implement constructor with std::vector<JsonValue> logic here
            }
            JsonValue(JsonArray&& value)
                : m_value(std::make_shared<JsonArray>(std::move(value)))
            {

            }
            JsonValue(const std::shared_ptr<JsonArray>& valuePtr)
                : m_value(valuePtr)
            {

            }
            JsonValue(std::shared_ptr<JsonArray>&& valuePtr) noexcept
                : m_value(std::move(valuePtr))
            {

            }

            // Constructor with std::map<std::string, JsonValue> value
            JsonValue(const JsonObject& value)
                : m_value(std::make_shared<JsonObject>(value))
                //, m_type(Type::Object)
                //, m_objElement(&std::get<JsonObject>(m_value))
            {
                // Implement constructor with std::map<std::string, JsonValue> logic here
            }

            // Move constructor with std::map<std::string, JsonValue> value
            JsonValue(JsonObject&& value) noexcept
                : m_value(std::make_shared<JsonObject>(std::move(value)))
                //, m_type(Type::Object)
                //, m_objElement(&std::get<JsonObject>(m_value))
            {
                // Implement move constructor with std::map<std::string, JsonValue> logic here
            }
            JsonValue(const std::shared_ptr<JsonObject>& valuePtr)
                : m_value(valuePtr)
            {

            }
            JsonValue(std::shared_ptr<JsonObject>&& valuePtr) noexcept
                : m_value(std::move(valuePtr))
            {

            }





            // Copy assignment operator
            JsonValue& operator=(const JsonValue& other)
            {
                m_value = other.m_value;
                //m_type = other.m_type;
                //if (m_type == Type::Object)
                //    m_objElement = &std::get<JsonObject>(m_value);
                //else
                //    m_objElement = nullptr;
                return *this;
            }

            // Move assignment operator
            JsonValue& operator=(JsonValue&& other) noexcept
            {
                m_value = std::move(other.m_value);
                //m_type = std::move(other.m_type);
                //if (m_type == Type::Object)
                //    m_objElement = &std::get<JsonObject>(m_value);
                //else
                //    m_objElement = nullptr;
                return *this;
            }

            // Assignment operator with std::string value
            JsonValue& operator=(const std::string& value)
            {
                m_value = value;
                //m_type = Type::String;
                //m_objElement = nullptr;
                return *this;
            }
            JsonValue& operator=(std::string&& value) noexcept
            {
                m_value = std::move(value);
                return *this;
            }

            // Assignment operator with const char* value
            JsonValue& operator=(const char* value)
            {
                m_value = std::string(value);
                //m_type = Type::String;
                //m_objElement = nullptr;
                return *this;
            }

            // Assignment operator with int value
            JsonValue& operator=(const int& value)
            {
                m_value = value;
                //m_type = Type::Int;
                //m_objElement = nullptr;
                return *this;
            }

            // Assignment operator with double value
            JsonValue& operator=(const double& value)
            {
                m_value = value;
                //m_type = Type::Double;
                //m_objElement = nullptr;
                return *this;
            }

            // Assignment operator with bool value
            JsonValue& operator=(const bool& value)
            {
                m_value = value;
                //m_type = Type::Bool;
                //m_objElement = nullptr;
                return *this;
            }

            // Assignment operator with JsonArray value
            JsonValue& operator=(const JsonArray& value)
            {
                std::shared_ptr<JsonArray>* ptr = std::get_if<std::shared_ptr<JsonArray>>(&m_value);
                if (ptr)
                {
                    ptr->get()->operator=(value);
                    return *this;
                }
                m_value = std::make_shared<JsonArray>(value);
                return *this;
            }

            // Assignment operator with JsonObject value
            JsonValue& operator=(const JsonObject& value)
            {
                std::shared_ptr<JsonObject>* ptr = std::get_if<std::shared_ptr<JsonObject>>(&m_value);
                if (ptr)
                {
                    ptr->get()->operator=(value);
                    return *this;
                }
                m_value = std::make_shared<JsonObject>(value);
                return *this;
            }

            JsonValue& operator=(const std::shared_ptr<JsonArray>& value)
            {
                m_value = value;
                return *this;
            }
            JsonValue& operator=(const std::shared_ptr<JsonObject>& value)
            {
                m_value = value;
                return *this;
            }

            // Move assignment operator with JsonObject value
            JsonValue& operator=(JsonObject&& value) noexcept
            {
                std::shared_ptr<JsonObject>* ptr = std::get_if<std::shared_ptr<JsonObject>>(&m_value);
                if (ptr)
                {
                    ptr->get()->operator=(std::move(value));
                    return *this;
                }
                m_value = std::make_shared<JsonObject>(std::move(value));
                //m_value = std::move(value);
                //m_type = Type::Object;
                //m_objElement = &std::get<JsonObject>(m_value);
                return *this;
            }
            JsonValue& operator=(JsonArray&& value) noexcept
            {
                std::shared_ptr<JsonArray>* ptr = std::get_if<std::shared_ptr<JsonArray>>(&m_value);
                if (ptr)
                {
                    ptr->get()->operator=(std::move(value));
                    return *this;
                }
                m_value = std::make_shared<JsonArray>(std::move(value));
                //m_value = std::move(value);
                //m_type = Type::Object;
                //m_objElement = &std::get<JsonObject>(m_value);
                return *this;
            }
            JsonValue& operator=(std::shared_ptr<JsonArray>&& value) noexcept
            {
                m_value = std::move(value);
                return *this;
            }
            JsonValue& operator=(std::shared_ptr<JsonObject>&& value) noexcept
            {
                m_value = std::move(value);
                return *this;
            }





            // Equality comparison operator
            bool operator==(const JsonValue& other) const
            {
                //if(m_type != other.m_type) return false;
                return m_value == other.m_value;
            }

            // Inequality comparison operator
            bool operator!=(const JsonValue& other) const
            {
                //if (m_type == other.m_type) return false;
                return !(*this == other);
            }



            // Convert value to string representation

            std::string toString() const
            {
                return serialize();
            }



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

		//std::string toString() const;
		std::string serialize() const;

		friend std::ostream& operator<<(std::ostream& os, const JsonValue& json);
		friend QDebug operator<<(QDebug debug, const JsonValue& json);

        // Overloading << operator for std::cout
        
	private:
		JsonVariantType m_value;
	};
}
#endif



namespace JsonDatabase
{

#if JD_ACTIVE_JSON == JD_JSON_INTERNAL
    // Default constructor
	/*
    JsonValue::JsonValue()
        : m_value(std::monostate{})
        // , m_type(Type::Null) 
        // , m_objElement(nullptr)
    {
        // Implement default constructor logic here
    }

    // Copy constructor
    JsonValue::JsonValue(const JsonValue& other)
        : m_value(other.m_value)
        // , m_type(other.m_type)
        // , m_objElement(nullptr)
    {
        // Implement copy constructor logic here
        //if (m_type == Type::Object)
        //    m_objElement = &std::get<JsonObject>(m_value);
    }

    // Move constructor
    JsonValue::JsonValue(JsonValue&& other) noexcept
        : m_value(std::move(other.m_value))
        // , m_type(std::move(other.m_type))
        // , m_objElement(nullptr)
    {
        //if (m_type == Type::Object)
        //    m_objElement = &std::get<JsonObject>(m_value);
        // Implement move constructor logic here
    }

    // Constructor with std::string value
    JsonValue::JsonValue(const std::string& value)
        : m_value(value)
        //, m_type(Type::String)
        //, m_objElement(nullptr)
    {
        // Implement constructor with std::string logic here
    }
    JsonValue::JsonValue(std::string&& value) noexcept
        : m_value(std::move(value))
    {

    }

    // Constructor with const char* value
    JsonValue::JsonValue(const char* value)
        : m_value(std::string(value))
        //, m_type(Type::String)
        //, m_objElement(nullptr)
    {
        // Implement constructor with const char* logic here
    }

    // Constructor with int value
    JsonValue::JsonValue(const int& value)
        : m_value(value)
        //, m_type(Type::Int)
        //, m_objElement(nullptr)
    {
        // Implement constructor with int logic here
    }

    // Constructor with double value
    JsonValue::JsonValue(const double& value)
        : m_value(value)
        //, m_type(Type::Double)
        //, m_objElement(nullptr)
    {
        // Implement constructor with double logic here
    }

    // Constructor with bool value
    JsonValue::JsonValue(const bool& value)
        : m_value(value)
        //, m_type(Type::Bool)
        //, m_objElement(nullptr)
    {
        // Implement constructor with bool logic here
    }

    // Constructor with std::vector<JsonValue> value
    JsonValue::JsonValue(const JsonArray& value)
        : m_value(std::make_shared<JsonArray>(value))
        //, m_type(Type::Array)
        //, m_objElement(nullptr)
    {
        // Implement constructor with std::vector<JsonValue> logic here
    }
    JsonValue::JsonValue(JsonArray&& value)
        : m_value(std::make_shared<JsonArray>(std::move(value)))
    {

    }
    JsonValue::JsonValue(const std::shared_ptr<JsonArray>& valuePtr)
        : m_value(valuePtr)
    {

    }
    JsonValue::JsonValue(std::shared_ptr<JsonArray>&& valuePtr) noexcept
        : m_value(std::move(valuePtr))
    {

    }

    // Constructor with std::map<std::string, JsonValue> value
    JsonValue::JsonValue(const JsonObject& value)
        : m_value(std::make_shared<JsonObject>(value))
        //, m_type(Type::Object)
        //, m_objElement(&std::get<JsonObject>(m_value))
    {
        // Implement constructor with std::map<std::string, JsonValue> logic here
    }

    // Move constructor with std::map<std::string, JsonValue> value
    JsonValue::JsonValue(JsonObject&& value) noexcept
        : m_value(std::make_shared<JsonObject>(std::move(value)))
        //, m_type(Type::Object)
        //, m_objElement(&std::get<JsonObject>(m_value))
    {
        // Implement move constructor with std::map<std::string, JsonValue> logic here
    }
    JsonValue::JsonValue(const std::shared_ptr<JsonObject>& valuePtr)
        : m_value(valuePtr)
    {

    }
    JsonValue::JsonValue(std::shared_ptr<JsonObject>&& valuePtr) noexcept
        : m_value(std::move(valuePtr))
    {

    }





    // Copy assignment operator
    JsonValue& JsonValue::operator=(const JsonValue& other)
    {
        m_value = other.m_value;
        //m_type = other.m_type;
        //if (m_type == Type::Object)
        //    m_objElement = &std::get<JsonObject>(m_value);
        //else
        //    m_objElement = nullptr;
        return *this;
    }

    // Move assignment operator
    JsonValue& JsonValue::operator=(JsonValue&& other) noexcept
    {
        m_value = std::move(other.m_value);
        //m_type = std::move(other.m_type);
        //if (m_type == Type::Object)
        //    m_objElement = &std::get<JsonObject>(m_value);
        //else
        //    m_objElement = nullptr;
        return *this;
    }

    // Assignment operator with std::string value
    JsonValue& JsonValue::operator=(const std::string& value)
    {
        m_value = value;
        //m_type = Type::String;
        //m_objElement = nullptr;
        return *this;
    }
    JsonValue& JsonValue::operator=(std::string&& value) noexcept
    {
        m_value = std::move(value);
        return *this;
    }

    // Assignment operator with const char* value
    JsonValue& JsonValue::operator=(const char* value)
    {
        m_value = std::string(value);
        //m_type = Type::String;
        //m_objElement = nullptr;
        return *this;
    }

    // Assignment operator with int value
    JsonValue& JsonValue::operator=(const int& value)
    {
        m_value = value;
        //m_type = Type::Int;
        //m_objElement = nullptr;
        return *this;
    }

    // Assignment operator with double value
    JsonValue& JsonValue::operator=(const double& value)
    {
        m_value = value;
        //m_type = Type::Double;
        //m_objElement = nullptr;
        return *this;
    }

    // Assignment operator with bool value
    JsonValue& JsonValue::operator=(const bool& value)
    {
        m_value = value;
        //m_type = Type::Bool;
        //m_objElement = nullptr;
        return *this;
    }

    // Assignment operator with JsonArray value
    JsonValue& JsonValue::operator=(const JsonArray& value)
    {
        std::shared_ptr<JsonArray>* ptr = std::get_if<std::shared_ptr<JsonArray>>(&m_value);
        if (ptr)
        {
            ptr->get()->operator=(value);
            return *this;
        }
        m_value = std::make_shared<JsonArray>(value);
        return *this;
    }

    // Assignment operator with JsonObject value
    JsonValue& JsonValue::operator=(const JsonObject& value)
    {
        std::shared_ptr<JsonObject>* ptr = std::get_if<std::shared_ptr<JsonObject>>(&m_value);
        if (ptr)
        {
            ptr->get()->operator=(value);
            return *this;
        }
        m_value = std::make_shared<JsonObject>(value);
        return *this;
    }

    JsonValue& JsonValue::operator=(const std::shared_ptr<JsonArray>& value)
    {
        m_value = value;
        return *this;
    }
    JsonValue& JsonValue::operator=(const std::shared_ptr<JsonObject>& value)
    {
        m_value = value;
        return *this;
    }

    // Move assignment operator with JsonObject value
    JsonValue& JsonValue::operator=(JsonObject&& value) noexcept
    {
        std::shared_ptr<JsonObject>* ptr = std::get_if<std::shared_ptr<JsonObject>>(&m_value);
        if (ptr)
        {
            ptr->get()->operator=(std::move(value));
            return *this;
        }
        m_value = std::make_shared<JsonObject>(std::move(value));
        //m_value = std::move(value);
        //m_type = Type::Object;
        //m_objElement = &std::get<JsonObject>(m_value);
        return *this;
    }
    JsonValue& JsonValue::operator=(JsonArray&& value) noexcept
    {
        std::shared_ptr<JsonArray>* ptr = std::get_if<std::shared_ptr<JsonArray>>(&m_value);
        if (ptr)
        {
            ptr->get()->operator=(std::move(value));
            return *this;
        }
        m_value = std::make_shared<JsonArray>(std::move(value));
        //m_value = std::move(value);
        //m_type = Type::Object;
        //m_objElement = &std::get<JsonObject>(m_value);
        return *this;
    }
    JsonValue& JsonValue::operator=(std::shared_ptr<JsonArray>&& value) noexcept
    {
        m_value = std::move(value);
        return *this;
    }
    JsonValue& JsonValue::operator=(std::shared_ptr<JsonObject>&& value) noexcept
    {
        m_value = std::move(value);
        return *this;
    }





    // Equality comparison operator
    bool JsonValue::operator==(const JsonValue& other) const
    {
        //if(m_type != other.m_type) return false;
        return m_value == other.m_value;
    }

    // Inequality comparison operator
    bool JsonValue::operator!=(const JsonValue& other) const
    {
        //if (m_type == other.m_type) return false;
        return !(*this == other);
    }



        // Convert value to string representation

    std::string JsonValue::toString() const
    {
        return serialize();
    }
    
	*/
    
#elif JD_ACTIVE_JSON == JD_JSON_GLAZE

    // Overloading << operator for std::cout
    std::ostream& operator<<(std::ostream& os, const JsonValue& json)
    {
        JsonSerializer serializer;
        std::string buff;
        serializer.serializeValue(json, buff);
        os << buff;
        return os;
    }
    std::ostream& operator<<(std::ostream& os, const JsonObject& json)
    {
        JsonSerializer serializer;
        std::string buff;
        serializer.serializeObject(json, buff);
        os << buff;
        return os;
    }
    std::ostream& operator<<(std::ostream& os, const JsonArray& json)
    {
        JsonSerializer serializer;
        std::string buff;
        serializer.serializeArray(json, buff);
        os << buff;
        return os;
    }

    // Overloading << operator for qDebug()
    QDebug operator<<(QDebug debug, const JsonValue& json)
    {
        QDebugStateSaver saver(debug);
        JsonSerializer serializer;
        std::string buff;
        serializer.serializeValue(json, buff);
        debug << buff.c_str();
        return debug;
    }
    QDebug operator<<(QDebug debug, const JsonObject& json)
    {
        QDebugStateSaver saver(debug);
        JsonSerializer serializer;
        std::string buff;
        serializer.serializeObject(json, buff);
        debug << buff.c_str();
        return debug;
    }
    QDebug operator<<(QDebug debug, const JsonArray& json)
    {
        QDebugStateSaver saver(debug);
        JsonSerializer serializer;
        std::string buff;
        serializer.serializeArray(json, buff);
        debug << buff.c_str();
        return debug;
    }
#endif
}

