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
			: m_isRegistered(false)
			, m_registryOpenTimeoutMs(100)
			, m_registeredUser(nullptr)
		{
			AbstractRegistry::setName("users");
		}
		JDUserRegistration::~JDUserRegistration()
		{
			unregisterUser();
		}


		bool JDUserRegistration::registerUser(const JDUser& user, std::string& generatedSessionIDOut)
		{
			m_registeredUser.reset();
			if (m_isRegistered)
				return false;

			if(!AbstractRegistry::openRegistryFile(m_registryOpenTimeoutMs))
				return false;
			AbstractRegistry::AutoClose autoClose(this);

			AbstractRegistry::removeInactiveObjects();

			std::string newSessionID = user.getSessionID();
			if (newSessionID.size() == 0)
				newSessionID = JDUser::generateSessionID();

			size_t tryCount = 0;
			while(AbstractRegistry::lockExists(newSessionID))
			{
				newSessionID = JDUser::generateSessionID();
				++tryCount;
			}
				
			JDUser newUser = user;
			newUser.setSessionID(newSessionID);
			auto newEntry = std::make_shared<LockEntryObjectImpl>(newUser.getSessionID(), newUser);
			m_registeredUser = newEntry;
			int ret = 0;
			while((ret = AbstractRegistry::addObjects({ newEntry })) != 1)
			{
				newSessionID = JDUser::generateSessionID();
				while (AbstractRegistry::lockExists(newSessionID))
				{
					newSessionID = JDUser::generateSessionID();
					++tryCount;
				}
				++tryCount;
				if (tryCount > 100)
					return false;
				newUser.setSessionID(newSessionID);
				newEntry = std::make_shared<LockEntryObjectImpl>(newUser.getSessionID(), newUser);
				m_registeredUser = newEntry;
			}

			generatedSessionIDOut  = newSessionID;
			m_registeredSessionID = newSessionID;
			m_isRegistered = true;
			return true;
		}
		bool JDUserRegistration::unregisterUser()
		{
			if (!m_isRegistered)
				return false;

			if (!AbstractRegistry::openRegistryFile(m_registryOpenTimeoutMs))
				return false;
			AbstractRegistry::AutoClose autoClose(this);

			if(AbstractRegistry::removeObjects({ m_registeredSessionID }) == 1)
			{
				m_isRegistered = false;
				m_registeredUser.reset();
				return true;
			}
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
			if (!AbstractRegistry::openRegistryFile(m_registryOpenTimeoutMs))
				return 0;
			AbstractRegistry::AutoClose autoClose(this);
			return AbstractRegistry::removeInactiveObjects();
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

#ifdef JD_USE_QJSON
		bool JDUserRegistration::LockEntryObjectImpl::load(const QJsonObject& obj) 
#else
		bool JDUserRegistration::LockEntryObjectImpl::load(const JsonObject& obj) 
#endif
		{
			bool success = LockEntryObject::load(obj);
#ifdef JD_USE_QJSON
			if(obj.contains(LockEntryObjectImpl::JsonKeys::user.c_str()))
			{
				QJsonValue userValue = obj[JsonKeys::user.c_str()];
#else
			if (obj.contains(LockEntryObjectImpl::JsonKeys::user))
			{
				JsonValue userValue = obj.at(JsonKeys::user);
#endif
				if(!userValue.isObject())
					return false;
				JDUser user;

#ifdef JD_USE_QJSON
				QJsonObject userObj = userValue.toObject();
#else
				JsonObject userObj = userValue.getObject();
#endif

				if (user.load(userObj))
				{
					setUser(user);
					success = true;
				}
			}
			return success;
		}

#ifdef JD_USE_QJSON
		bool JDUserRegistration::LockEntryObjectImpl::save(QJsonObject& obj) const
#else
		bool JDUserRegistration::LockEntryObjectImpl::save(JsonObject& obj) const
#endif
		{
			bool success = LockEntryObject::save(obj);
#ifdef JD_USE_QJSON
			QJsonObject userData;
#else
			JsonObject userData;
#endif
			m_user.save(userData);
#ifdef JD_USE_QJSON
			obj[JsonKeys::user.c_str()] = userData;
#else
			obj[JsonKeys::user] = userData;
#endif
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