#include "Json/JsonValue.h"
#if JD_ACTIVE_JSON == JD_JSON_INTERNAL 
#include "Json/JsonSerializer.h"
#include <QDebug>

#endif


namespace JsonDatabase
{
#if JD_ACTIVE_JSON == JD_JSON_INTERNAL

    
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

    std::string JsonValue::serialize() const
    {
        JsonSerializer serializer;
        return serializer.serializeValue(*this);
    }

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

#endif
}