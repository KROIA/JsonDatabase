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

			return false;
		}
		bool JDUserRegistration::unregisterUser(const JDUser& user)
		{

			return false;
		}

		bool JDUserRegistration::isUserRegistered(const JDUser& user) const
		{

			return false;
		}
		bool JDUserRegistration::isUserRegistered(const std::string& sessionID) const
		{

			return false;
		}

		std::vector<JDUser> JDUserRegistration::getRegisteredUsers() const
		{

			return std::vector<JDUser>();
		}
		int JDUserRegistration::unregisterInactiveUsers()
		{

			return 0;
		}
	}
}