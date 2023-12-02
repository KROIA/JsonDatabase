#include "utilities/JDUserRegistration.h"

namespace JsonDatabase
{
	namespace Utilities
	{


		JDUserRegistration::JDUserRegistration(JDManager& manager)
			: m_manager(manager)
		{

		}
		JDUserRegistration::~JDUserRegistration()
		{

		}

		bool JDUserRegistration::registerUser(const JDUser& user)
		{

		}
		bool JDUserRegistration::unregisterUser(const JDUser& user)
		{

		}

		bool JDUserRegistration::isUserRegistered(const JDUser& user) const
		{

		}
		bool JDUserRegistration::isUserRegistered(const std::string& sessionID) const
		{

		}

		std::vector<JDUser> JDUserRegistration::getRegisteredUsers() const
		{

		}
		int JDUserRegistration::unregisterInactiveUsers()
		{

		}
	}
}