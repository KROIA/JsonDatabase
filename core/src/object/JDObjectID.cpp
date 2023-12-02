#include "object/JDObjectID.h"
#include "utilities/JDObjectIDDomain.h"


namespace JsonDatabase
{
    const JDObjectID::IDType JDObjectID::invalidID = 0; // Invalid ID

    // Constructors
    JDObjectID::JDObjectID(const IDType& id, 
                           State state,
                           const std::shared_ptr<JDObjectIDDomainInterface> &domain)
        : m_id(id)
        , m_isValid(state)
        , m_domainInterface(domain)
    {
        if (m_id == invalidID)
            m_isValid = State::Invalid;
    }

    bool JDObjectID::isValid(const JDObjectIDptr& id)
    {
        if (!id.get())
            return false;
        return id->isValid();
    }
   /* JDObjectID::JDObjectID()
        : m_id(invalidID)
        , m_isValid(State::Invalid)
        , m_domainInterface(nullptr)
    {

    }*/
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

    /*JDObjectID::JDObjectID(const IDType& id)
        : m_id(id) 
        , m_isValid(true)
        , m_domainInterface(other.m_domainInterface
    {
        if(id == invalidID)
			m_isValid = false;
        // Implement constructor from unsigned int directly initializing m_id
    }
*/
   /*
    JDObjectID::JDObjectID(const JDObjectID& other) 
        : m_id(other.m_id) 
        , m_isValid(other.m_isValid)
        , m_domainInterface(other.m_domainInterface)
    {
        // Implement copy constructor
    }

    JDObjectID::JDObjectID(const JDObjectID&& other) noexcept 
        : m_id(std::move(other.m_id))
        , m_isValid(std::move(other.m_isValid))
        , m_domainInterface(std::move(other.m_domainInterface))
    {
        // Implement move constructor
    }

    // Assignment operator
    JDObjectID& JDObjectID::operator=(const JDObjectID& other) {
        // Implement assignment operator
        m_id = other.m_id;
        m_isValid = other.m_isValid;
        return *this;
    }
    */

    // Comparison operators
    bool JDObjectID::operator==(const JDObjectID& other) const {
        // Implement ==
        return m_id == other.m_id && 
               m_isValid == other.m_isValid;
    }

    bool JDObjectID::operator!=(const JDObjectID& other) const {
        // Implement !=
        return m_id != other.m_id || 
               m_isValid != other.m_isValid;
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
    bool JDObjectID::isValid() const 
    {
        return m_isValid == State::Valid;
    }

    const JDObjectID::IDType& JDObjectID::get() const {
        // Implement get() - Return a const reference to the ID
        return m_id;
    }

    std::string JDObjectID::toString() const {
        // Implement toString() - Convert m_id to a string and return it
        return toString(m_id);
    }

    QString JDObjectID::toQString() const {
        // Implement toQString() - Convert m_id to a QString and return it
        return toQString(m_id);
    }

    std::string JDObjectID::toString(const IDType& id)
    {
        return std::to_string(id);
    }

    QString JDObjectID::toQString(const IDType& id)
    {
        return QString::number(id);
    }

    bool JDObjectID::unregister()
    {
        if (m_domainInterface)
		{
            JDObjectIDptr ptr = JDObjectIDptr(this);
			m_domainInterface->unregisterID(ptr);
			m_domainInterface.reset();
			return true;
		}
		return false;
    }

    // Static functions
   /* JDObjectID JDObjectID::nullID() {
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
    }*/


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