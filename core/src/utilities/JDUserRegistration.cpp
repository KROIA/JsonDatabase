#include "utilities/JDUserRegistration.h"
#include "manager/JDManager.h"
#include "utilities/filesystem/FileLock.h"
#include <QDir>


namespace JsonDatabase
{
	namespace Utilities
	{
		const std::string JDUserRegistration::LockEntryObjectImpl::JsonKeys::user = "user";

		JDUserRegistration::JDUserRegistration()
			: AbstractRegistry()
			, m_isRegistered(false)
			, m_registryOpenTimeoutMs(100)
			, m_registeredUser(nullptr)
		{
			AbstractRegistry::setName("users");
		}
		JDUserRegistration::~JDUserRegistration()
		{
			unregisterUser();
			delete m_logger;
		}


		bool JDUserRegistration::registerUser(const JDUser& user, std::string& generatedSessionIDOut)
		{
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
			m_registeredUser.reset();
			if (m_isRegistered)
				return false;

			if(!AbstractRegistry::openRegistryFile(m_registryOpenTimeoutMs))
				goto failed;

			{
				AbstractRegistry::AutoClose autoClose(this);

				AbstractRegistry::removeInactiveObjects();

				std::string newSessionID = user.getSessionID();
				if (newSessionID.size() == 0)
					newSessionID = JDUser::generateSessionID();

				size_t tryCount = 0;
				while (AbstractRegistry::lockExists(newSessionID))
				{
					newSessionID = JDUser::generateSessionID();
					++tryCount;
				}

				JDUser newUser = user;
				newUser.setSessionID(newSessionID);
				auto newEntry = std::make_shared<LockEntryObjectImpl>(newUser.getSessionID(), newUser);
				m_registeredUser = newEntry;
				int ret = 0;
				while ((ret = AbstractRegistry::addObjects({ newEntry })) != 1)
				{
					newSessionID = JDUser::generateSessionID();
					while (AbstractRegistry::lockExists(newSessionID))
					{
						newSessionID = JDUser::generateSessionID();
						++tryCount;
					}
					++tryCount;
					if (tryCount > 100)
						goto failed;
					newUser.setSessionID(newSessionID);
					newEntry = std::make_shared<LockEntryObjectImpl>(newUser.getSessionID(), newUser);
					m_registeredUser = newEntry;
				}

				generatedSessionIDOut = newSessionID;
				m_registeredSessionID = newSessionID;
				m_isRegistered = true;

				if (m_logger)
					m_logger->logInfo("Registered user: " + user.toString());
				return true;
			}

		failed:
			if(m_logger)
				m_logger->logError("Failed to register user: "+ user.toString());
			return false;
		}
		bool JDUserRegistration::unregisterUser()
		{
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
			if (!m_isRegistered)
				return false;

			if (!AbstractRegistry::openRegistryFile(m_registryOpenTimeoutMs))
				goto failed;
			{
				AbstractRegistry::AutoClose autoClose(this);

				if (AbstractRegistry::removeObjects({ m_registeredSessionID }) == 1)
				{
					m_isRegistered = false;
					if (m_logger)
						m_logger->logInfo("Unregistered user: " + m_registeredUser->getUser().toString());
					m_registeredUser.reset();
					return true;
				}
			}
		failed:
			if (m_logger)
				m_logger->logError("Failed to unregister user: " + m_registeredUser->getUser().toString());
			return false;
		}

		bool JDUserRegistration::isUserRegistered() const
		{
			return m_isRegistered;
		}
		bool JDUserRegistration::isUserRegistered(const JDUser& user) const
		{
			return AbstractRegistry::isObjectActive(user.getSessionID());
		}
		bool JDUserRegistration::isUserRegistered(const std::string& sessionID) const
		{
			return AbstractRegistry::isObjectActive(sessionID);
		}


		std::vector<JDUser> JDUserRegistration::getRegisteredUsers() const
		{
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
			if (!AbstractRegistry::openRegistryFile(m_registryOpenTimeoutMs))
				return std::vector<JDUser>();
			AbstractRegistry::AutoClose autoClose(this);

			std::vector<std::shared_ptr<LockEntryObjectImpl>> userObjs; 
			AbstractRegistry::readObjects<LockEntryObjectImpl>(userObjs);

			std::vector<JDUser> users;
			for (auto& userObj : userObjs)
			{
				users.push_back(userObj->getUser());
			}
			return users;
		}
		int JDUserRegistration::unregisterInactiveUsers() const
		{
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
			
			if (!AbstractRegistry::openRegistryFile(m_registryOpenTimeoutMs))
				return 0;
			AbstractRegistry::AutoClose autoClose(this);
			return AbstractRegistry::removeInactiveObjects();
		}

		bool JDUserRegistration::checkForUserChange(std::vector<JDUser>& loggedOnUsers, std::vector<JDUser>& loggedOffUsers) const
		{
			std::vector<JDUser> currentUsers = getRegisteredUsers();

			loggedOnUsers.clear();
			loggedOffUsers.clear();

			for (auto& user : currentUsers)
			{
				bool found = false;
				for (auto& lastUser : m_lastActiveUsers)
				{
					if (user.getSessionID() == lastUser.getSessionID())
					{
						found = true;
						break;
					}
				}
				if (!found)
				{
					loggedOnUsers.push_back(user);
					if (m_logger)
						m_logger->logInfo("User logged on: " + user.toString());
				}
			}
			for (auto& lastUser : m_lastActiveUsers)
			{
				bool found = false;
				for (auto& user : currentUsers)
				{
					if (user.getSessionID() == lastUser.getSessionID())
					{
						found = true;
						break;
					}
				}
				if (!found)
				{
					loggedOffUsers.push_back(lastUser);
					if (m_logger)
						m_logger->logInfo("User logged off: " + lastUser.toString());
				}
			}
			m_lastActiveUsers = currentUsers;
			return loggedOffUsers.size() > 0 || loggedOnUsers.size() > 0;
		}


		



		JDUserRegistration::LockEntryObjectImpl::LockEntryObjectImpl(const std::string& key)
			: LockEntryObject(key)
		{

		}
		JDUserRegistration::LockEntryObjectImpl::LockEntryObjectImpl(const std::string& key, const JDUser& user)
			: LockEntryObject(key)
			, m_user(user)
		{

		}
		JDUserRegistration::LockEntryObjectImpl::~LockEntryObjectImpl()
		{

		}


		void JDUserRegistration::LockEntryObjectImpl::setUser(const JDUser& user)
		{
			m_user = user;
		}
		const JDUser& JDUserRegistration::LockEntryObjectImpl::getUser() const
		{
			return m_user;
		}

		bool JDUserRegistration::LockEntryObjectImpl::load(const JsonObject& obj) 
		{
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
			bool success = LockEntryObject::load(obj);
			if (obj.contains(LockEntryObjectImpl::JsonKeys::user))
			{
				const auto& userValue = obj.at(JsonKeys::user);

				if(!userValue.holds<JsonObject>())
					return false;
				JDUser user;

				const JsonObject &userObj = userValue.get<JsonObject>();

				if (user.load(userObj))
				{
					setUser(user);
					success = true;
				}
			}
			return success;
		}
		bool JDUserRegistration::LockEntryObjectImpl::save(JsonObject& obj) const
		{
			JD_REGISTRY_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
			bool success = LockEntryObject::save(obj);
			JsonObject userData;
			m_user.save(userData);
			obj[JsonKeys::user] = userData;
			return success;
		}

		void JDUserRegistration::onCreateFiles()
		{

		}

		void JDUserRegistration::onDatabasePathChangeStart(const std::string& newPath)
		{

			JD_UNUSED(newPath);
		}
		void JDUserRegistration::onDatabasePathChangeEnd()
		{

		}

		void JDUserRegistration::onNameChange(const std::string& newName)
		{
			JD_UNUSED(newName);
		}
	}
}