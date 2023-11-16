#include "Json/JsonValue.h"
#include "Json/JsonSerializer.h"


namespace JsonDatabase
{
    // Default constructor
    JsonValue::JsonValue() 
        : m_value(std::monostate{}) 
        , m_type(Type::Null) 
    {
        // Implement default constructor logic here
    }

    // Copy constructor
    JsonValue::JsonValue(const JsonValue& other) 
        : m_value(other.m_value) 
        , m_type(other.m_type)
    {
        // Implement copy constructor logic here
    }

    // Move constructor
    JsonValue::JsonValue(JsonValue&& other) noexcept 
        : m_value(std::move(other.m_value)) 
        , m_type(std::move(other.m_type))
    {
        // Implement move constructor logic here
    }

    // Constructor with std::string value
    JsonValue::JsonValue(const std::string& value) 
        : m_value(value) 
        , m_type(Type::String)
    {
        // Implement constructor with std::string logic here
    }

    // Constructor with const char* value
    JsonValue::JsonValue(const char* value) 
        : m_value(std::string(value)) 
        , m_type(Type::String)
    {
        // Implement constructor with const char* logic here
    }

    // Constructor with int value
    JsonValue::JsonValue(const int& value) 
        : m_value(value) 
        , m_type(Type::Int)
    {
        // Implement constructor with int logic here
    }

    // Constructor with double value
    JsonValue::JsonValue(const double& value) 
        : m_value(value) 
        , m_type(Type::Double)
    {
        // Implement constructor with double logic here
    }

    // Constructor with bool value
    JsonValue::JsonValue(const bool& value) 
        : m_value(value) 
        , m_type(Type::Bool)
    {
        // Implement constructor with bool logic here
    }

    // Constructor with std::vector<JsonValue> value
    JsonValue::JsonValue(const JsonArray& value) 
        : m_value(value) 
        , m_type(Type::Array)
    {
        // Implement constructor with std::vector<JsonValue> logic here
    }

    // Constructor with std::map<std::string, JsonValue> value
    JsonValue::JsonValue(const JsonObject& value) 
        : m_value(value) 
        , m_type(Type::Object)
    {
        // Implement constructor with std::map<std::string, JsonValue> logic here
    }

    // Move constructor with std::map<std::string, JsonValue> value
    JsonValue::JsonValue(JsonObject&& value) noexcept 
        : m_value(std::move(value))
        , m_type(Type::Object)
    {
        // Implement move constructor with std::map<std::string, JsonValue> logic here
    }





    // Copy assignment operator
    JsonValue& JsonValue::operator=(const JsonValue& other) 
    {
        m_value = other.m_value;
        m_type = other.m_type;
        return *this;
    }

    // Move assignment operator
    JsonValue& JsonValue::operator=(JsonValue&& other) noexcept 
    {
        m_value = std::move(other.m_value);
        m_type = std::move(other.m_type);
        return *this;
    }

    // Assignment operator with std::string value
    JsonValue& JsonValue::operator=(const std::string& value) 
    {
        m_value = value;
        m_type = Type::String;
        return *this;
    }

    // Assignment operator with const char* value
    JsonValue& JsonValue::operator=(const char* value)
    {
        m_value = std::string(value);
        m_type = Type::String;
        return *this;
    }

    // Assignment operator with int value
    JsonValue& JsonValue::operator=(const int& value)
    {
        m_value = value;
        m_type = Type::Int;
        return *this;
    }

    // Assignment operator with double value
    JsonValue& JsonValue::operator=(const double& value)
    {
        m_value = value;
        m_type = Type::Double;
        return *this;
    }

    // Assignment operator with bool value
    JsonValue& JsonValue::operator=(const bool& value) 
    {
        m_value = value;
        m_type = Type::Bool;
        return *this;
    }

    // Assignment operator with JsonArray value
    JsonValue& JsonValue::operator=(const JsonArray& value) 
    {
        m_value = value;
        m_type = Type::Array;
        return *this;
    }

    // Assignment operator with JsonObject value
    JsonValue& JsonValue::operator=(const JsonObject& value)
    {
        m_value = value;
        m_type = Type::Object;
        return *this;
    }

    // Move assignment operator with JsonObject value
    JsonValue& JsonValue::operator=(JsonObject&& value) noexcept 
    {
        m_value = std::move(value);
        m_type = Type::Object;
        return *this;
    }





    // Equality comparison operator
    bool JsonValue::operator==(const JsonValue& other) const 
    {
        if(m_type != other.m_type) return false;
        return m_value == other.m_value;
    }

    // Inequality comparison operator
    bool JsonValue::operator!=(const JsonValue& other) const 
    {
        if (m_type == other.m_type) return false;
        return !(*this == other);
    }

    // Less than comparison operator
    bool JsonValue::operator<(const JsonValue& other) const 
    {
        return std::visit([](
            const std::variant<std::monostate, std::string, int, double, bool, JsonArray, JsonObject>& lhs, 
            const std::variant<std::monostate, std::string, int, double, bool, JsonArray, JsonObject>& rhs) 
            {
                using T = std::decay_t<decltype(lhs)>;
                if constexpr (std::is_same_v<T, std::monostate>) {
                    // Handle comparison for std::monostate
                    return false; // For this example, consider std::monostate as always equal
                }
                else if constexpr (std::is_same_v<T, std::string>) {
                    // Handle comparison for std::string
                    return (std::get<std::string>(lhs) < std::get<std::string>(rhs));
                }
                else if constexpr (std::is_same_v<T, int>) {
                    // Handle comparison for int
                    return (std::get<int>(lhs) < std::get<int>(rhs));
                }
                else if constexpr (std::is_same_v<T, double>) {
                    // Handle comparison for double
                    return (std::get<double>(lhs) < std::get<double>(rhs));
                }
                else if constexpr (std::is_same_v<T, bool>) {
                    // Handle comparison for bool
                    return (std::get<bool>(lhs) < std::get<bool>(rhs));
                }
                else if constexpr (std::is_same_v<T, JsonArray>) {
                    // Handle comparison for JsonArray
                    // Assuming JsonArray does not support comparison for simplicity
                    throw std::logic_error("Comparison not supported for JsonArray");
                }
                else if constexpr (std::is_same_v<T, JsonObject>) {
                    // Handle comparison for JsonObject
                    // Assuming JsonObject does not support comparison for simplicity
                    throw std::logic_error("Comparison not supported for JsonObject");
                }
            }, m_value, other.m_value);
    }

    // Greater than comparison operator
    bool JsonValue::operator>(const JsonValue& other) const 
    {
        return other < *this;
    }

    // Less than or equal to comparison operator
    bool JsonValue::operator<=(const JsonValue& other) const 
    {
        return !(other < *this);
    }

    // Greater than or equal to comparison operator
    bool JsonValue::operator>=(const JsonValue& other) const 
    {
        return !(*this < other);
    }


    JsonValue::Type JsonValue::getType() const
    {
		return m_type;
    }
    // Check if value is null
    bool JsonValue::isNull() const 
    {
        return m_type == Type::Null;
    }

    // Check if value is a string
    bool JsonValue::isString() const 
    {
        return m_type == Type::String;
    }

    // Check if value is a number (either int or double)
    bool JsonValue::isNumber() const
    {
        return m_type == Type::Int || m_type == Type::Double;
    }

    // Check if value is an int
    bool JsonValue::isInt() const 
    {
        return m_type == Type::Int;
    }

    // Check if value is a double
    bool JsonValue::isDouble() const 
    {
        return m_type == Type::Double;
    }

    // Check if value is a boolean
    bool JsonValue::isBool() const 
    {
        return m_type == Type::Bool;
    }

    // Check if value is an array
    bool JsonValue::isArray() const 
    {
        return m_type == Type::Array;
    }

    // Check if value is an object
    bool JsonValue::isObject() const
    {
        return m_type == Type::Object;
    }

    // Convert value to string representation
    std::string JsonValue::toString() const 
    {
        return JsonSerializer::serializeValue(*this);
    }




}