#pragma once

#include "JsonDatabase_base.h"
#include "JsonDatabase_Declaration.h"
#include <string>
#include <QString>
#include <QDebug>
#include <ostream>
#include <memory>

namespace JsonDatabase
{
	
	class JSON_DATABASE_EXPORT JDObjectID
	{
		friend JDObjectIDDomain;
	public:

#define JD_ID_TYPE_STRING 1
#define JD_ID_TYPE_LONG 2

// Select the type for a ID here:
#define JD_ID_TYPE_SWITCH JD_ID_TYPE_LONG
//#define JD_ID_TYPE_SWITCH JD_ID_TYPE_STRING



#if JD_ID_TYPE_SWITCH == JD_ID_TYPE_STRING
		using IDType = std::string;
#elif JD_ID_TYPE_SWITCH == JD_ID_TYPE_LONG
		using IDType = long;
#else
#error "Invalid ID type"
#endif

		static const IDType invalidID;
		static const IDType defaultID;


		enum State
		{
			Invalid,
			Valid
		};
	private:
		JDObjectID(const IDType& id, 
				   State state,
				   const std::shared_ptr<JDObjectIDDomainInterface> &domain);
	public:
		

		//JDObjectID();
		//JDObjectID(const QString& id);
		//JDObjectID(const std::string &id);
		//JDObjectID(const IDType &id);
		//JDObjectID(const JDObjectID &id);
		
		JDObjectID(const JDObjectID& other) = delete;
		JDObjectID(const JDObjectID&& other) = delete;

		static bool isValid(const JDObjectIDptr& id);

		JDObjectID& operator=(const JDObjectID& other) = delete;

		bool operator==(const JDObjectID& other) const;
		bool operator!=(const JDObjectID& other) const;
		bool operator<(const JDObjectID& other) const;
		bool operator>(const JDObjectID& other) const;
		bool operator<=(const JDObjectID& other) const;
		bool operator>=(const JDObjectID& other) const;

		bool isValid() const;

		const IDType& get() const;

		std::string toString() const;
		static std::string toString(const IDType& id);
		QString toQString() const;
		static QString toQString(const IDType& id);

		bool unregister();

		// static JDObjectID nullID();
		// static JDObjectID generateID();

		static std::string idToStr(const JDObjectID::IDType& id);

		friend std::ostream& operator<<(std::ostream& os, const JDObjectID& id);
		friend QDebug operator<<(QDebug debug, const JDObjectID& id);
	private:
		// Invalid value for all types of ID

		static std::string toStringInternal(int ID) {
			return std::to_string(ID);
		}
		static const std::string &toStringInternal(const std::string& ID) {
			return ID;
		}


		IDType m_id;
		State m_isValid;
		std::shared_ptr<JDObjectIDDomainInterface> m_domainInterface;
	};
}
