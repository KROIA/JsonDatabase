#pragma once

#include "JD_base.h"
#include <string>
#include <QString>
#include <QDebug>
#include <ostream>

namespace JsonDatabase
{
	class JSONDATABASE_EXPORT JDObjectID
	{
	public:
		using IDType = int;

		JDObjectID();
		//JDObjectID(const QString& id);
		//JDObjectID(const std::string &id);
		JDObjectID(const IDType &id);
		
		JDObjectID(const JDObjectID& other);
		JDObjectID(const JDObjectID&& other) noexcept;

		JDObjectID& operator=(const JDObjectID& other);

		bool operator==(const JDObjectID& other) const;
		bool operator!=(const JDObjectID& other) const;
		bool operator<(const JDObjectID& other) const;
		bool operator>(const JDObjectID& other) const;
		bool operator<=(const JDObjectID& other) const;
		bool operator>=(const JDObjectID& other) const;

		bool isValid() const;
		bool isNull() const;

		const IDType& get() const;

		std::string toString() const;
		QString toQString() const;

		static JDObjectID nullID();
		static JDObjectID generateID();

		friend std::ostream& operator<<(std::ostream& os, const JDObjectID& id);
		friend QDebug operator<<(QDebug debug, const JDObjectID& id);
	private:

		IDType m_id;
	};
}