#pragma once

#include "JD_base.h"
#include "JDSerializable.h"
#include <string>
#include <QDateTime>
#include "json/JsonValue.h"


namespace JsonDatabase
{
	namespace Utilities
	{
		class JSONDATABASE_EXPORT JDUser : public JDSerializable
		{
		public:
			JDUser();
			JDUser( const std::string& sessionID, 
					const std::string& name, 
					const QTime& loginTime, 
					const QDate& loginDate);
			JDUser(const JDUser& other);
			JDUser(JDUser&& other) noexcept;
			JDUser& operator=(const JDUser& other);

			~JDUser();



			const std::string &getSessionID() const;
			void setSessionID(const std::string &sessionID);

			const std::string &getName() const;
			void setName(const std::string &name);

			const QTime &getLoginTime() const;
			const QDate &getLoginDate() const;

			void setLoginTime(const QTime &loginTime);
			void setLoginDate(const QDate &loginDate);

			bool operator==(const JDUser& other) const;
			bool operator!=(const JDUser& other) const;

			// Compares the login time and date
			bool operator<(const JDUser& other) const;
			bool operator>(const JDUser& other) const;
			bool operator<=(const JDUser& other) const;
			bool operator>=(const JDUser& other) const;

			std::string toString() const;


			bool load(const JsonObject& obj) override;
			bool save(JsonObject& obj) const override;


			struct JsonKeys
			{
				static const std::string sessionID;
				static const std::string name;
					    
				static const std::string date;
				static const std::string time;
			};

			static std::string getHostName();
			static std::string generateSessionID();
			static JDUser generateUser();
			static JDUser generateUser(const std::string &name);
			static JDUser generateUser(const std::string &name, const std::string &sessionID);
		private:
			
			std::string m_sessionID;
			std::string m_name;

			QDate m_loginDate;
			QTime m_loginTime;

			static constexpr unsigned int s_sessionIDLength = 32;
		};
	}
}