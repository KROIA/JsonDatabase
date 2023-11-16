#include "object/JDObjectID.h"


namespace JsonDatabase
{

    // Constructors
    JDObjectID::JDObjectID() 
    {
        m_id = 0;
    }
    /*JDObjectID::JDObjectID(const QString& id) {
        // Implement constructor from QString
        // Convert QString to unsigned int and store it in m_id
        m_id = id.toUInt();
    }

    JDObjectID::JDObjectID(const std::string &id) {
        // Implement constructor from std::string
        // Convert std::string to unsigned int and store it in m_id
        m_id = std::stoi(id);
    }*/

    JDObjectID::JDObjectID(const IDType &id) 
        : m_id(id) 
    {
        // Implement constructor from unsigned int directly initializing m_id
    }

    JDObjectID::JDObjectID(const JDObjectID& other) 
        : m_id(other.m_id) 
    {
        // Implement copy constructor
    }

    JDObjectID::JDObjectID(const JDObjectID&& other) noexcept 
        : m_id(std::move(other.m_id)) 
    {
        // Implement move constructor
    }

    // Assignment operator
    JDObjectID& JDObjectID::operator=(const JDObjectID& other) {
        // Implement assignment operator
        m_id = other.m_id;
        return *this;
    }

    // Comparison operators
    bool JDObjectID::operator==(const JDObjectID& other) const {
        // Implement ==
        return m_id == other.m_id;
    }

    bool JDObjectID::operator!=(const JDObjectID& other) const {
        // Implement !=
        return m_id != other.m_id;
    }

    bool JDObjectID::operator<(const JDObjectID& other) const {
        // Implement <
        return m_id < other.m_id;
    }

    bool JDObjectID::operator>(const JDObjectID& other) const {
        // Implement >
        return m_id > other.m_id;
    }

    bool JDObjectID::operator<=(const JDObjectID& other) const {
        // Implement <=
        return m_id <= other.m_id;
    }

    bool JDObjectID::operator>=(const JDObjectID& other) const {
        // Implement >=
        return m_id >= other.m_id;
    }

    // Utility functions
    bool JDObjectID::isValid() const {
        // Implement isValid() - Return true if the ID is valid (specific condition based on your requirement)
        // Example: return m_id > 0;
        // This function checks if the ID is valid according to your specific rules
        return m_id > 0;
    }

    bool JDObjectID::isNull() const {
        // Implement isNull() - Return true if the ID is null (specific condition based on your requirement)
        // Example: return m_id == 0;
        // This function checks if the ID represents null according to your specific rules
        return m_id == 0;
    }

    const JDObjectID::IDType& JDObjectID::get() const {
        // Implement get() - Return a const reference to the ID
        return m_id;
    }

    std::string JDObjectID::toString() const {
        // Implement toString() - Convert m_id to a string and return it
        return std::to_string(m_id);
    }

    QString JDObjectID::toQString() const {
        // Implement toQString() - Convert m_id to a QString and return it
        return QString::number(m_id);
    }

    // Static functions
    JDObjectID JDObjectID::nullID() {
        // Implement nullID() - Return a JDObjectID representing null (specific value based on your requirement)
        // Example: return JDObjectID(0);
        // This function returns a predefined null JDObjectID according to your specific rules
        return JDObjectID(0);
    }

    JDObjectID JDObjectID::generateID() {
        // Implement generateID() - Generate a new JDObjectID (specific generation logic based on your requirement)
        // Example: return JDObjectID(some_logic_to_generate_id);
        // This function generates a new JDObjectID according to your specific rules
        return JDObjectID(1);
    }


    // Overloading << operator for std::cout
    std::ostream& operator<<(std::ostream& os, const JDObjectID& id) 
    {
        os << id.toString(); // Using toString() function to get the string representation of the ID
        return os;
    }

    // Overloading << operator for qDebug()
    QDebug operator<<(QDebug debug, const JDObjectID& id) 
    {
        QDebugStateSaver saver(debug);
        debug << id.toString().c_str(); // Using toString() function to get the string representation of the ID
        return debug;
    }
}