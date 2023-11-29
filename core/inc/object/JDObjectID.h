#pragma once

#include "JD_base.h"
#include "JDDeclaration.h"
#include <string>
#include <QString>
#include <QDebug>
#include <ostream>
#include <memory>

namespace JsonDatabase
{
	using JDObjectIDptr = std::shared_ptr<JDObjectID>;

	class JSONDATABASE_EXPORT JDObjectID
	{
		friend JDObjectIDDomain;
	public:
		using IDType = int;
		static const IDType invalidID;

		enum State
		{
			Invalid,
			Valid
		};
	private:
		JDObjectID(const IDType& id, 
				   State state,
				   std::shared_ptr<JDObjectIDDomainInterface> domain);
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

		friend std::ostream& operator<<(std::ostream& os, const JDObjectID& id);
		friend QDebug operator<<(QDebug debug, const JDObjectID& id);
	private:

		IDType m_id;
		State m_isValid;
		std::shared_ptr<JDObjectIDDomainInterface> m_domainInterface;
	};
}