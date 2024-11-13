#include "utilities/JDUser.h"
#include "utilities/JDUtilities.h"


#include "Json/JsonSerializer.h"

namespace JsonDatabase
{
	namespace Utilities
	{

        const std::string JDUser::JsonKeys::sessionID = "sessionID";
        const std::string JDUser::JsonKeys::name = "name";
                          
        const std::string JDUser::JsonKeys::date = "date";
        const std::string JDUser::JsonKeys::time = "time";

        // Default constructor
        JDUser::JDUser() {}

        // Parameterized constructor
        JDUser::JDUser(const std::string& sessionID,
            const std::string& name,
            const QTime& loginTime,
            const QDate& loginDate)
            : m_sessionID(sessionID)
            , m_name(name)
            , m_loginTime(loginTime)
            , m_loginDate(loginDate) 
        {}

        // Copy constructor
        JDUser::JDUser(const JDUser& other)
            : m_sessionID(other.m_sessionID)
            , m_name(other.m_name)
            , m_loginTime(other.m_loginTime)
            , m_loginDate(other.m_loginDate)
        {}

        // Move constructor
        JDUser::JDUser(JDUser&& other) noexcept
            : m_sessionID(std::move(other.m_sessionID))
            , m_name(std::move(other.m_name))
            , m_loginTime(std::move(other.m_loginTime))
            , m_loginDate(std::move(other.m_loginDate)) 
        {}

        // Copy assignment operator
        JDUser& JDUser::operator=(const JDUser& other) {
            if (this != &other) {
                m_sessionID = other.m_sessionID;
                m_name = other.m_name;
                m_loginTime = other.m_loginTime;
                m_loginDate = other.m_loginDate;
            }
            return *this;
        }

        // Destructor
        JDUser::~JDUser() {}

        // Getter for sessionID
        const std::string& JDUser::getSessionID() const {
            return m_sessionID;
        }

        // Setter for sessionID
        void JDUser::setSessionID(const std::string& sessionID) {
            m_sessionID = sessionID;
        }

        // Getter for name
        const std::string& JDUser::getName() const {
            return m_name;
        }

        // Setter for name
        void JDUser::setName(const std::string& name) {
            m_name = name;
        }

        // Getter for loginTime
        const QTime& JDUser::getLoginTime() const {
            return m_loginTime;
        }

        // Getter for loginDate
        const QDate& JDUser::getLoginDate() const {
            return m_loginDate;
        }

        // Setter for loginTime
        void JDUser::setLoginTime(const QTime& loginTime) {
            m_loginTime = loginTime;
        }

        // Setter for loginDate
        void JDUser::setLoginDate(const QDate& loginDate) {
            m_loginDate = loginDate;
        }

        // Comparison operators
        bool JDUser::operator==(const JDUser& other) const {
            return (m_sessionID == other.m_sessionID &&
                m_name == other.m_name &&
                m_loginTime == other.m_loginTime &&
                m_loginDate == other.m_loginDate);
        }

        bool JDUser::operator!=(const JDUser& other) const {
            return !(*this == other);
        }

        bool JDUser::operator<(const JDUser& other) const {
            // Implement comparison based on login time and date
            // Example comparison, you may need to adjust based on your requirements
            return (m_loginDate < other.m_loginDate ||
                   (m_loginDate == other.m_loginDate && m_loginTime < other.m_loginTime));
        }

        bool JDUser::operator>(const JDUser& other) const {
            return (m_loginDate > other.m_loginDate ||
                   (m_loginDate == other.m_loginDate && m_loginTime > other.m_loginTime));
        }

        bool JDUser::operator<=(const JDUser& other) const {
            return (m_loginDate < other.m_loginDate ||
                   (m_loginDate == other.m_loginDate && m_loginTime <= other.m_loginTime));
        }

        bool JDUser::operator>=(const JDUser& other) const {
            return (m_loginDate > other.m_loginDate ||
                   (m_loginDate == other.m_loginDate && m_loginTime >= other.m_loginTime));
        }

        std::string JDUser::toString() const
        {
            JsonObject obj;
			save(obj);
            JsonSerializer serializer;
			return serializer.serializeObject(obj);
        }

        // Implement load and save functions based on JD_USE_QJSON flag
        bool JDUser::load(const JsonObject& obj) 
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
            bool success = true;
            auto sessionIt = obj.find(JsonKeys::sessionID);
            auto nameIt = obj.find(JsonKeys::name);
            auto dateIt = obj.find(JsonKeys::date);
            auto timeIt = obj.find(JsonKeys::time);

            if (sessionIt == obj.end() || !sessionIt->second.holds<std::string>())
                success = false;

            if (nameIt == obj.end() || !nameIt->second.holds<std::string>())
                success = false;

            if (dateIt == obj.end() || !dateIt->second.holds<std::string>())
                success = false;

            if (timeIt == obj.end() || !timeIt->second.holds<std::string>())
                success = false;

            if (!success)
                return false;

            m_sessionID = sessionIt->second.get<std::string>();
            m_name = nameIt->second.get<std::string>();

            m_loginDate = fastStringToQDate(dateIt->second.get<std::string>());
            m_loginTime = fastStringToQTime(timeIt->second.get<std::string>());

            return success;
        }

        bool JDUser::save(JsonObject& obj) const 
        {
            //obj.reserve(5);
            *obj[JsonKeys::sessionID] = m_sessionID;
            *obj[JsonKeys::name] = m_name;

            *obj[JsonKeys::date] = qDateToString(m_loginDate);
            *obj[JsonKeys::time] = qTimeToString(m_loginTime);
            return true; 
        }


        std::string JDUser::getHostName()
        {
			// Get hostname using QT
            QString name = qgetenv("USER");
            if (name.isEmpty())
                name = qgetenv("USERNAME");
            return name.toStdString();
        }
        std::string JDUser::generateSessionID()
        {
            return Utilities::generateRandomString(s_sessionIDLength);
        }
        JDUser JDUser::generateUser()
        {
            return generateUser(getHostName());
        }
        JDUser JDUser::generateUser(const std::string& name)
        {
			return generateUser(name, generateSessionID());
        }
        JDUser JDUser::generateUser(const std::string& name, const std::string& sessionID)
        {
            JDUser user;
            user.setName(name);
            user.setSessionID(sessionID);
            user.setLoginDate(QDate::currentDate());
            user.setLoginTime(QTime::currentTime());
            return user;
        }
	}
}