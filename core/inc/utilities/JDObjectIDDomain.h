#pragma once

#include "JD_base.h"
#include "JDDeclaration.h"
#include "object/JDObjectID.h"
#include <unordered_map>
#include <string>
#include <memory>

namespace JsonDatabase
{
	class JSONDATABASE_EXPORT JDObjectIDDomainInterface
	{
		friend JDObjectIDDomain;
		JDObjectIDDomainInterface(JDObjectIDDomain* domain);
		

		void setDomain(JDObjectIDDomain* domain);
		JDObjectIDDomain* getDomainInternal();

	public:
		~JDObjectIDDomainInterface();

		const JDObjectIDDomain *getDomain() const;
		bool isDomainAlive() const;

		bool unregisterID(JDObjectIDptr& id);

	private:
		JDObjectIDDomain* m_domain;
	};

	class JSONDATABASE_EXPORT JDObjectIDDomain
	{
	public:
		JDObjectIDDomain();
		JDObjectIDDomain(const std::string &name);
		JDObjectIDDomain(const JDObjectIDDomain& other) = delete;
		JDObjectIDDomain(JDObjectIDDomain&& other) = delete;

		~JDObjectIDDomain();

		JDObjectIDDomain& operator=(const JDObjectIDDomain& other) = delete;
		JDObjectIDDomain& operator=(JDObjectIDDomain&& other) = delete;


		void setName(const std::string &name);
		const std::string& getName() const;

		/*
			Generates a new ID unique to this domain.
		*/
		JDObjectIDptr getNewID();

		/*
			Checks if the given ID is unique to this domain.
			If it is, success is set to true and the ID is returned.
			If it is not, success is set to false and a invalid ID is generated and returned.
		*/
		JDObjectIDptr getPredefinedID(const JDObjectID::IDType &existing, bool &success);
		std::vector<JDObjectIDptr> getPredefinedIDs(const std::vector<JDObjectID::IDType> &existingIDs, bool &allSuccess);
		
		JDObjectIDptr getExistingID(const JDObjectID::IDType &existing) const;
		

		bool idExists(const JDObjectID::IDType &id) const;
		bool idExists(const JDObjectIDptr &id) const;

		bool unregisterID(const JDObjectID::IDType &id);
		bool unregisterID(JDObjectIDptr id);

	private:
		void generateNextID(unsigned int increment = 1);

		std::string m_name;
		std::shared_ptr<JDObjectIDDomainInterface> m_interface;

		static const JDObjectID::IDType s_startID;
		JDObjectID::IDType m_nextID;
		std::unordered_map<JDObjectID::IDType, JDObjectIDptr> m_usedIDs;
	};
}