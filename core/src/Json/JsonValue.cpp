#include "Json/JsonValue.h"
#if JD_ACTIVE_JSON == JD_JSON_INTERNAL || JD_ACTIVE_JSON == JD_JSON_GLAZE
#include "Json/JsonSerializer.h"
#include <QDebug>

#endif



namespace JsonDatabase
{

#if JD_ACTIVE_JSON == JD_JSON_INTERNAL
    // Default constructor
    JsonValue::JsonValue() 
        : m_value(std::monostate{}) 
        , m_type(Type::Null) 
        , m_objElement(nullptr)
    {
        // Implement default constructor logic here
    }

    // Copy constructor
    JsonValue::JsonValue(const JsonValue& other) 
        : m_value(other.m_value) 
        , m_type(other.m_type)
        , m_objElement(nullptr)
    {
        // Implement copy constructor logic here
        if (m_type == Type::Object)
            m_objElement = &std::get<JsonObject>(m_value);
    }

    // Move constructor
    JsonValue::JsonValue(JsonValue&& other) noexcept 
        : m_value(std::move(other.m_value)) 
        , m_type(std::move(other.m_type))
        , m_objElement(nullptr)
    {
        if (m_type == Type::Object)
            m_objElement = &std::get<JsonObject>(m_value);
        // Implement move constructor logic here
    }

    // Constructor with std::string value
    JsonValue::JsonValue(const std::string& value) 
        : m_value(value) 
        , m_type(Type::String)
        , m_objElement(nullptr)
    {
        // Implement constructor with std::string logic here
    }

    // Constructor with const char* value
    JsonValue::JsonValue(const char* value) 
        : m_value(std::string(value)) 
        , m_type(Type::String)
        , m_objElement(nullptr)
    {
        // Implement constructor with const char* logic here
    }

    // Constructor with int value
    JsonValue::JsonValue(const int& value) 
        : m_value(value) 
        , m_type(Type::Int)
        , m_objElement(nullptr)
    {
        // Implement constructor with int logic here
    }

    // Constructor with double value
    JsonValue::JsonValue(const double& value) 
        : m_value(value) 
        , m_type(Type::Double)
        , m_objElement(nullptr)
    {
        // Implement constructor with double logic here
    }

    // Constructor with bool value
    JsonValue::JsonValue(const bool& value) 
        : m_value(value) 
        , m_type(Type::Bool)
        , m_objElement(nullptr)
    {
        // Implement constructor with bool logic here
    }

    // Constructor with std::vector<JsonValue> value
    JsonValue::JsonValue(const JsonArray& value) 
        : m_value(value) 
        , m_type(Type::Array)
        , m_objElement(nullptr)
    {
        // Implement constructor with std::vector<JsonValue> logic here
    }

    // Constructor with std::map<std::string, JsonValue> value
    JsonValue::JsonValue(const JsonObject& value) 
        : m_value(value) 
        , m_type(Type::Object)
        , m_objElement(&std::get<JsonObject>(m_value))
    {
        // Implement constructor with std::map<std::string, JsonValue> logic here
    }

    // Move constructor with std::map<std::string, JsonValue> value
    JsonValue::JsonValue(JsonObject&& value) noexcept 
        : m_value(std::move(value))
        , m_type(Type::Object)
        , m_objElement(&std::get<JsonObject>(m_value))
    {
        // Implement move constructor with std::map<std::string, JsonValue> logic here
    }

    JsonValue::~JsonValue()
    {
        
    }



    // Copy assignment operator
    JsonValue& JsonValue::operator=(const JsonValue& other) 
    {
        m_value = other.m_value;
        m_type = other.m_type;
        if (m_type == Type::Object)
            m_objElement = &std::get<JsonObject>(m_value);
        else
            m_objElement = nullptr;
        return *this;
    }

    // Move assignment operator
    JsonValue& JsonValue::operator=(JsonValue&& other) noexcept 
    {
        m_value = std::move(other.m_value);
        m_type = std::move(other.m_type);
        if (m_type == Type::Object)
            m_objElement = &std::get<JsonObject>(m_value);
        else
            m_objElement = nullptr;
        return *this;
    }

    // Assignment operator with std::string value
    JsonValue& JsonValue::operator=(const std::string& value) 
    {
        m_value = value;
        m_type = Type::String;
        m_objElement = nullptr;
        return *this;
    }

    // Assignment operator with const char* value
    JsonValue& JsonValue::operator=(const char* value)
    {
        m_value = std::string(value);
        m_type = Type::String;
        m_objElement = nullptr;
        return *this;
    }

    // Assignment operator with int value
    JsonValue& JsonValue::operator=(const int& value)
    {
        m_value = value;
        m_type = Type::Int;
        m_objElement = nullptr;
        return *this;
    }

    // Assignment operator with double value
    JsonValue& JsonValue::operator=(const double& value)
    {
        m_value = value;
        m_type = Type::Double;
        m_objElement = nullptr;
        return *this;
    }

    // Assignment operator with bool value
    JsonValue& JsonValue::operator=(const bool& value) 
    {
        m_value = value;
        m_type = Type::Bool;
        m_objElement = nullptr;
        return *this;
    }

    // Assignment operator with JsonArray value
    JsonValue& JsonValue::operator=(const JsonArray& value) 
    {
        m_value = value;
        m_type = Type::Array;
        m_objElement = nullptr;
        return *this;
    }

    // Assignment operator with JsonObject value
    JsonValue& JsonValue::operator=(const JsonObject& value)
    {
        m_value = value;
        m_type = Type::Object;
        m_objElement = &std::get<JsonObject>(m_value);
        return *this;
    }

    // Move assignment operator with JsonObject value
    JsonValue& JsonValue::operator=(JsonObject&& value) noexcept 
    {
        m_value = std::move(value);
        m_type = Type::Object;
        m_objElement = &std::get<JsonObject>(m_value);
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

    /*
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
    */

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

    bool JsonValue::contains(const std::string& key) const
    {
        if (m_type != Type::Object)
            return false;
		return m_objElement->contains(key);
    }
    JsonValue& JsonValue::operator[](const std::string& key)
    {
        if (m_type != Type::Object)
        {
            static JsonValue nullValue;
            nullValue = JsonValue();
            return nullValue;
        }
        return (*m_objElement)[key];
    }


    std::string& JsonValue::getString()
    {
        return std::get<std::string>(m_value);
    }
    int& JsonValue::getInt()
    {
        return std::get<int>(m_value);
    }
    double& JsonValue::getDouble()
    {
        return std::get<double>(m_value);
    }
    bool& JsonValue::getBool()
    {
        return std::get<bool>(m_value);
    }
    JsonArray& JsonValue::getArray()
    {
        return std::get<JsonArray>(m_value);
    }
    JsonObject& JsonValue::getObject()
    {
        return *m_objElement;
    }

    const std::string& JsonValue::getString() const
    {
        return std::get<std::string>(m_value);
    }
    const int& JsonValue::getInt() const
    {
        return std::get<int>(m_value);
    }
    const double& JsonValue::getDouble() const
    {
        return std::get<double>(m_value);
    }
    const bool& JsonValue::getBool() const
    {
        return std::get<bool>(m_value);
    }
    const JsonArray& JsonValue::getArray() const
    {
        return std::get<JsonArray>(m_value);
    }
    const JsonObject& JsonValue::getObject() const
    {
        return *m_objElement;
    }

    bool JsonValue::extractString(std::string& valueOut) const
    {
        if(m_type != Type::String)
			return false;
        valueOut = std::get<std::string>(m_value);
		return true;
    }
    /*bool JsonValue::getString(JsonValue& objOut, const std::string& key) const
    {
        if (m_type != Type::Object)
			return false;
        const auto &it = m_objElement->find(key);
        if (it == m_objElement->end())
			return false;
        objOut = it->second;
        return true;
    }*/
    bool JsonValue::extractInt(int& valueOut) const
    {
        if (m_type != Type::Int)
            return false;
        valueOut = std::get<int>(m_value);
        return true;
    }
    bool JsonValue::extractDouble(double& valueOut) const
    {
        if (m_type != Type::Double)
			return false;
		valueOut = std::get<double>(m_value);
		return true;
    }
    bool JsonValue::extractBool(bool& valueOut) const
    {
		if (m_type != Type::Bool)
            return false;
        valueOut = std::get<bool>(m_value);
        return true;
    }
    bool JsonValue::extractArray(JsonArray& valueOut) const
    {
        if (m_type != Type::Array)
			return false;

		valueOut = std::get<JsonArray>(m_value);
		return true;
    }
    bool JsonValue::extractObject(JsonObject& valueOut) const
    {
        if (m_type != Type::Object)
            return false;
        valueOut = *m_objElement;
        return true;
    }

    bool JsonValue::extractString(std::string& valueOut, const std::string& key) const
    {
        if (m_type != Type::Object)
            return false;
        const auto& it = m_objElement->find(key);
        if (it == m_objElement->end())
            return false;
        valueOut = it->second.getString();
        return true;
    }
    bool JsonValue::extractInt(int& valueOut, const std::string& key) const
    {
        if (m_type != Type::Object)
			return false;
        const auto& it = m_objElement->find(key);
        if (it == m_objElement->end())
            return false;
        valueOut = it->second.getInt();
		return true;
    }
    bool JsonValue::extractDouble(double& valueOut, const std::string& key) const
    {
        if (m_type != Type::Object)
            return false;
        const auto& it = m_objElement->find(key);
        if (it == m_objElement->end())
            return false;
        valueOut = it->second.getDouble();
        return true;
    }
    bool JsonValue::extractBool(bool& valueOut, const std::string& key) const
    {
        if (m_type != Type::Object)
			return false;
        const auto& it = m_objElement->find(key);
        if (it == m_objElement->end())
            return false;
        valueOut = it->second.getBool();
        return true;
    }
    bool JsonValue::extractArray(JsonArray& valueOut, const std::string& key) const
    {
        if (m_type != Type::Object)
            return false;
        const auto& it = m_objElement->find(key);
        if (it == m_objElement->end())
            return false;
        valueOut = std::get<JsonArray>(it->second.m_value);
        return true;
    }
    bool JsonValue::extractObject(JsonObject& valueOut, const std::string& key) const
    {
        if (m_type != Type::Object)
			return false;
        const auto& it = m_objElement->find(key);
        if (it == m_objElement->end())
            return false;
        valueOut = std::get<JsonObject>(it->second.m_value);
		return true;
    }

    std::string& JsonValue::getString(const std::string& key)
    {
        const auto& it = m_objElement->find(key);
        return std::get<std::string>(it->second.m_value);
    }
    int& JsonValue::getInt(const std::string& key)
    {
        const auto& it = m_objElement->find(key);
        return std::get<int>(it->second.m_value);
    }
    double& JsonValue::getDouble(const std::string& key)
    {
        const auto& it = m_objElement->find(key);
        return std::get<double>(it->second.m_value);
    }
    bool& JsonValue::getBool(const std::string& key)
    {
        const auto& it = m_objElement->find(key);
        return std::get<bool>(it->second.m_value);
    }
    JsonArray& JsonValue::getArray(const std::string& key)
    {
        const auto& it = m_objElement->find(key);
        return std::get<JsonArray>(it->second.m_value);
    }
    JsonObject& JsonValue::getObject(const std::string& key)
    {
        const auto& it = m_objElement->find(key);
        return *(it->second.m_objElement);
    }

    


    const std::string& JsonValue::getString(const std::string& key) const
    {
        const auto& it = m_objElement->find(key);
        return std::get<std::string>(it->second.m_value);
    }
    const int& JsonValue::getInt(const std::string& key) const
    {
        const auto& it = m_objElement->find(key);
        return std::get<int>(it->second.m_value);
    }
    const double& JsonValue::getDouble(const std::string& key) const
    {
        const auto& it = m_objElement->find(key);
        return std::get<double>(it->second.m_value);
    }
    const bool& JsonValue::getBool(const std::string& key) const
    {
        const auto& it = m_objElement->find(key);
        return std::get<bool>(it->second.m_value);
    }
    const JsonArray& JsonValue::getArray(const std::string& key) const
    {
        const auto& it = m_objElement->find(key);
        return std::get<JsonArray>(it->second.m_value);
    }
    const JsonObject& JsonValue::getObject(const std::string& key) const
    {
        const auto& it = m_objElement->find(key);
        return *(it->second.m_objElement);
    }

    std::string* JsonValue::getStringPtr(const std::string& key)
    {
        if (!m_objElement) return nullptr;
        const auto& it = m_objElement->find(key);
        return &std::get<std::string>(it->second.m_value);
    }
    int* JsonValue::getIntPtr(const std::string& key)
    {
        if (!m_objElement) return nullptr;
        const auto& it = m_objElement->find(key);
        return &std::get<int>(it->second.m_value);
    }
    double* JsonValue::getDoublePtr(const std::string& key)
    {
        if (!m_objElement) return nullptr;
        const auto& it = m_objElement->find(key);
        return &std::get<double>(it->second.m_value);
    }
    bool* JsonValue::getBoolPtr(const std::string& key)
    {
        if (!m_objElement) return nullptr;
        const auto& it = m_objElement->find(key);
        return &std::get<bool>(it->second.m_value);
    }
    JsonArray* JsonValue::getArrayPtr(const std::string& key)
    {
        if (!m_objElement) return nullptr;
        const auto& it = m_objElement->find(key);
        return &std::get<JsonArray>(it->second.m_value);
    }
    JsonObject* JsonValue::getObjectPtr(const std::string& key)
    {
        if (!m_objElement) return nullptr;
        const auto& it = m_objElement->find(key);
        return it->second.m_objElement;
    }

    const std::string* JsonValue::getStringPtr(const std::string& key) const
    {
        if (!m_objElement) return nullptr;
        const auto& it = m_objElement->find(key);
        return &std::get<std::string>(it->second.m_value);
    }
    const int* JsonValue::getIntPtr(const std::string& key) const
    {
        if (!m_objElement) return nullptr;
        const auto& it = m_objElement->find(key);
        return &std::get<int>(it->second.m_value);
    }
    const double* JsonValue::getDoublePtr(const std::string& key) const
    {
        if (!m_objElement) return nullptr;
        const auto& it = m_objElement->find(key);
        return &std::get<double>(it->second.m_value);
    }
    const bool* JsonValue::getBoolPtr(const std::string& key) const
    {
        if (!m_objElement) return nullptr;
        const auto& it = m_objElement->find(key);
        return &std::get<bool>(it->second.m_value);
    }
    const JsonArray* JsonValue::getArrayPtr(const std::string& key) const
    {
        if (!m_objElement) return nullptr;
        const auto& it = m_objElement->find(key);
        return &std::get<JsonArray>(it->second.m_value);
    }
    const JsonObject* JsonValue::getObjectPtr(const std::string& key) const
    {
        if (!m_objElement) return nullptr;
        const auto& it = m_objElement->find(key);
        return it->second.m_objElement;
    }


    JsonValue::JsonVariantType& JsonValue::getVariant()
    {
        return m_value;
    }
    const JsonValue::JsonVariantType& JsonValue::getConstVariant() const
    {
        return m_value;
    }

    // Convert value to string representation
    std::string JsonValue::toString() const 
    {
        return serialize();
    }
    std::string JsonValue::serialize() const
    {
        JsonSerializer serializer;
		return serializer.serializeValue(*this);
    }

    // Overloading << operator for std::cout
    std::ostream& operator<<(std::ostream& os, const JsonValue& json)
    {
        os << json.toString();
        return os;
    }

    // Overloading << operator for qDebug()
    QDebug operator<<(QDebug debug, const JsonValue& json)
    {
        QDebugStateSaver saver(debug);
        debug << json.toString().c_str(); 
        return debug;
    }
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
