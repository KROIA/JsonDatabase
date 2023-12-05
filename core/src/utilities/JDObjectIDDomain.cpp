#include "utilities/JDObjectIDDomain.h"

namespace JsonDatabase
{

	JDObjectIDDomainInterface::JDObjectIDDomainInterface(JDObjectIDDomain* domain)
		: m_domain(domain)
	{

	}

	JDObjectIDDomainInterface::~JDObjectIDDomainInterface()
	{
		m_domain = nullptr;
	}


	void JDObjectIDDomainInterface::setDomain(JDObjectIDDomain* domain)
	{
		m_domain = domain;
	}

	JDObjectIDDomain* JDObjectIDDomainInterface::getDomainInternal()
	{
		return m_domain;
	}


	const JDObjectIDDomain* JDObjectIDDomainInterface::getDomain() const
	{
		return m_domain;
	}
	bool JDObjectIDDomainInterface::isDomainAlive() const
	{
		return m_domain != nullptr;
	}

	bool JDObjectIDDomainInterface::unregisterID(JDObjectIDptr& id)
	{
		if (!m_domain)
			return false; // Domain is not alive
		m_domain->unregisterID(id);
		return true;
	}






	const JDObjectID::IDType JDObjectIDDomain::s_startID = 1;

	JDObjectIDDomain::JDObjectIDDomain()
		: m_name("")
		, m_interface(std::shared_ptr<JDObjectIDDomainInterface>(new JDObjectIDDomainInterface(this)))
		, m_nextID(s_startID)
	{

	};
	JDObjectIDDomain::JDObjectIDDomain(const std::string& name)
		: m_name(name)
		, m_interface(std::shared_ptr<JDObjectIDDomainInterface>(new JDObjectIDDomainInterface(this)))
		, m_nextID(s_startID)
	{

	};


	JDObjectIDDomain::~JDObjectIDDomain()
	{
		m_interface->setDomain(nullptr);
	}


	void JDObjectIDDomain::setName(const std::string& name)
	{
		m_name = name;	
	}
	const std::string& JDObjectIDDomain::getName() const
	{
		return m_name;
	}

	JDObjectIDptr JDObjectIDDomain::getNewID()
	{
		JD_ID_DOMAIN_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
		auto it = m_usedIDs.find(m_nextID);

		unsigned int counter = 1;
		while(it != m_usedIDs.end())
		{
			// Generate new ID until we find one that is not used
			generateNextID(counter);
			counter++;
			it = m_usedIDs.find(m_nextID);
		}

		JDObjectIDptr id = JDObjectIDptr( new JDObjectID(m_nextID, JDObjectID::State::Valid, m_interface));
		m_usedIDs[m_nextID] = id;
		generateNextID();
		return id;
	}

	

	JDObjectIDptr JDObjectIDDomain::getPredefinedID(const JDObjectID::IDType& existing, bool& success)
	{
		JD_ID_DOMAIN_PROFILING_FUNCTION(JD_COLOR_STAGE_1);

		if(existing == JDObjectID::invalidID)
			goto invalid;

		{
			auto it = m_usedIDs.find(existing);
			if (it != m_usedIDs.end())
				goto invalid;
		}

		{
			JDObjectIDptr id = JDObjectIDptr(new JDObjectID(existing, JDObjectID::State::Valid, m_interface));
			m_usedIDs[existing] = id;
			success = true;
			return id;
		}

	    invalid:
		success = false;
		return nullptr;
	}
	std::vector<JDObjectIDptr> JDObjectIDDomain::getPredefinedIDs(const std::vector<JDObjectID::IDType>& existingIDs, bool& allSuccess)
	{
		JD_ID_DOMAIN_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
		std::vector<JDObjectIDptr> ids;
		allSuccess = true;


		m_usedIDs.reserve(m_usedIDs.size() + existingIDs.size());
		ids.reserve(existingIDs.size());
		for (rsize_t i = 0; i < existingIDs.size(); ++i)
		{
			const JDObjectID::IDType& current = existingIDs[i];
			if (current == JDObjectID::invalidID)
			{
				allSuccess = false;
				ids.push_back(nullptr);
				continue;
			}
			else
			{
				auto it = m_usedIDs.find(current);
				if (it != m_usedIDs.end())
				{
					allSuccess = false;
					ids.push_back(nullptr);
					continue;
				}
			}
			JDObjectIDptr id = JDObjectIDptr(new JDObjectID(current, JDObjectID::State::Valid, m_interface));
			m_usedIDs[current] = id;
			ids.push_back(id);
		}
		return ids;
	}
	JDObjectIDptr JDObjectIDDomain::getExistingID(const JDObjectID::IDType& existing) const
	{
		JD_ID_DOMAIN_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
		if (existing == JDObjectID::invalidID)
			return nullptr;

		auto it = m_usedIDs.find(existing);
		if (it != m_usedIDs.end())
			return it->second;
		return nullptr;
	
	}

	bool JDObjectIDDomain::idExists(const JDObjectID::IDType& id) const
	{
		auto it = m_usedIDs.find(id);
		if(it != m_usedIDs.end())
		{
			if (it->second)
				return true;
		}
		return false;
	}
	bool JDObjectIDDomain::idExists(const JDObjectIDptr& id) const
	{
		return idExists(id->get());
	}

	bool JDObjectIDDomain::unregisterID(const JDObjectID::IDType& id)
	{
		auto it = m_usedIDs.find(id);
		if (it != m_usedIDs.end())
		{
			if (it->second)
			{
				it->second->m_isValid = JDObjectID::State::Invalid;
				it->second->m_id = JDObjectID::invalidID;
				it->second = nullptr;
				m_usedIDs.erase(it);
				return true;	
			}
		}
		return false;
	}
	bool JDObjectIDDomain::unregisterID(JDObjectIDptr id)
	{
		if(id->m_domainInterface != m_interface)
			return false; // ID does not belong to this domain

		bool success = unregisterID(id->get());
		return success;
	}

	void JDObjectIDDomain::generateNextID(unsigned int increment)
	{
		m_nextID += increment;
	}
}