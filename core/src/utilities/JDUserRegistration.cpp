#include "utilities/JDUserRegistration.h"
#include "manager/JDManager.h"
#include "utilities/filesystem/FileLock.h"
#include <QDir>


namespace JsonDatabase
{
	namespace Utilities
	{


		JDUserRegistration::JDUserRegistration(JDManager& manager)
			: m_manager(manager)
			, m_registeredFileLock(nullptr)
			, m_registrationSubFolder("users")
			, m_registrationFileName("users")
			, m_registrationFileEnding(".json")
		{

		}
		JDUserRegistration::~JDUserRegistration()
		{
			unregisterUser();
		}

		bool JDUserRegistration::setup()
		{
			std::string path = getPath();
			QDir dir1(path.c_str());
			if (!dir1.exists())
			{
				QDir dir2;
				if (!dir2.mkdir(path.c_str()))
				{
					JD_DEBUG("JDUserRegistration::setup() - Failed to create directory: " + getPath());
					return false;
				}
			}
			QFile file1(getRegisterFilePath().c_str());
			if(!file1.exists())
			{
				Internal::LockedFileAccessor registerFile(path, m_registrationFileName, m_registrationFileEnding);
				Internal::LockedFileAccessor::Error err;
				if ((err = registerFile.lock(Internal::LockedFileAccessor::AccessMode::write)) != Internal::LockedFileAccessor::Error::none)
				{
					return false;
				}
				if ((err = registerFile.writeJsonFile(std::vector<QJsonObject>())) != Internal::LockedFileAccessor::Error::none)
				{
					JD_DEBUG("JDUserRegistration::setup() - Failed to write to file: " + getRegisterFilePath() + " Error: " + registerFile.getErrorStr(err));
					return false;
				}
			}

			return true;
		}

		bool JDUserRegistration::registerUser(const JDUser& user, std::string& generatedSessionIDOut)
		{
			if (m_registeredFileLock)
				return false;

			Internal::LockedFileAccessor::Error err;
			Internal::LockedFileAccessor registerFile(getPath(), m_registrationFileName, m_registrationFileEnding);
			err = registerFile.lock(Internal::LockedFileAccessor::AccessMode::readWrite, 100);
			if (err != Internal::LockedFileAccessor::Error::none)
			{
				return false;
			}

			unregisterInactiveUsers_internal(registerFile);

			std::vector<JDUser> activeUsers = getRegisteredUsers_internal(registerFile);
			std::string generatedSessionID = user.getSessionID();
			std::unordered_map<std::string, JDUser> userMap;
			for (auto& user : activeUsers)
			{
				userMap[user.getSessionID()] = user;
			}
			
			Internal::FileLock::Error lockErr;
			do
			{
				if(m_registeredFileLock)
				{
					delete m_registeredFileLock;
					m_registeredFileLock = nullptr;
				}
				// Generate new session ID if the generated one is already in use
				while (userMap.find(generatedSessionID) != userMap.end())
				{
					generatedSessionID = JDUser::generateSessionID();
				}

				m_registeredFileLock = new Internal::FileLock(getPath(), generatedSessionID);
				m_registeredFileLock->lock(lockErr);

			} while (lockErr != Internal::FileLock::Error::none);
			m_registeredSessionID = generatedSessionID;


			JDUser newUser = user;
			newUser.setSessionID(generatedSessionID);
			generatedSessionIDOut = generatedSessionID;
			activeUsers.push_back(newUser);
#ifdef JD_USE_QJSON
			std::vector<QJsonObject> userJsons;
#else
			JsonArray userJsons;
#endif
			for (auto& user : activeUsers)
			{
#ifdef JD_USE_QJSON
				QJsonObject obj;
#else
				JsonValue obj;
#endif
				user.save(obj);
				userJsons.push_back(obj);
			}
			
			err = registerFile.writeJsonFile(userJsons);
			if (err != Internal::LockedFileAccessor::Error::none)
			{
				return false;
			}
			return true;
		}
		bool JDUserRegistration::unregisterUser()
		{
			return unregisterUser_internal();
		}

		bool JDUserRegistration::isUserRegistered() const
		{
			return isSessionIDActive(m_registeredSessionID);
		}
		bool JDUserRegistration::isUserRegistered(const JDUser& user) const
		{
			return isSessionIDActive(user.getSessionID());
		}
		bool JDUserRegistration::isUserRegistered(const std::string& sessionID) const
		{
			return isSessionIDActive(sessionID);
		}


		std::vector<JDUser> JDUserRegistration::getRegisteredUsers() const
		{
			Internal::LockedFileAccessor::Error err;
			Internal::LockedFileAccessor registerFile(getPath(), m_registrationFileName, m_registrationFileEnding);
			err = registerFile.lock(Internal::LockedFileAccessor::AccessMode::read, 100);
			if (err != Internal::LockedFileAccessor::Error::none)
			{
				return std::vector<JDUser>();
			}

			std::vector<JDUser> activeUsers = getRegisteredUsers_internal(registerFile);
			return activeUsers;
		}
		int JDUserRegistration::unregisterInactiveUsers() const 
		{
			Internal::LockedFileAccessor::Error err;
			Internal::LockedFileAccessor registerFile(getPath(), m_registrationFileName, m_registrationFileEnding);
			err = registerFile.lock(Internal::LockedFileAccessor::AccessMode::readWrite, 100);
			if (err != Internal::LockedFileAccessor::Error::none)
			{
				return 0;
			}
			return unregisterInactiveUsers_internal(registerFile);
			
		}


		


		std::vector<JDUser> JDUserRegistration::getRegisteredUsers_internal(Internal::LockedFileAccessor& registerFile) const
		{
			std::vector<JDUser> users;

#ifdef JD_USE_QJSON
			std::vector<QJsonObject> userJsons;
#else
			JsonArray userJsons;
#endif
			Internal::LockedFileAccessor::Error err;
			err = registerFile.readJsonFile(userJsons);
			if (err != Internal::LockedFileAccessor::Error::none)
			{
				return users;
			}
			for (auto& userJson : userJsons)
			{
				JDUser user;
				if (user.load(userJson))
				{
					if (isSessionIDActive(user.getSessionID()))
						users.push_back(user);
				}
			}

			return users;
		} 
		bool JDUserRegistration::unregisterUser_internal()
		{
			if (!m_registeredFileLock)
				return false;
			Internal::LockedFileAccessor::Error err;
			Internal::LockedFileAccessor registerFile(getPath(), m_registrationFileName, m_registrationFileEnding);
			err = registerFile.lock(Internal::LockedFileAccessor::AccessMode::readWrite, 100);
			if (err != Internal::LockedFileAccessor::Error::none)
			{
				return false;
			}

			std::vector<JDUser> activeUsers = getRegisteredUsers_internal(registerFile);
			for(size_t i=0; i<activeUsers.size(); ++i)
			{
				if (activeUsers[i].getSessionID() == m_registeredSessionID)
				{
					activeUsers.erase(activeUsers.begin() + i);
					break;
				}
			}
			#ifdef JD_USE_QJSON
			std::vector<QJsonObject> userJsons;
			#else
			JsonArray userJsons;
			#endif
			for (auto& user : activeUsers)
			{
				#ifdef JD_USE_QJSON
				QJsonObject obj;
				#else
				JsonValue obj;
				#endif
				user.save(obj);
				userJsons.push_back(obj);
			}
			err = registerFile.writeJsonFile(userJsons);

			Internal::FileLock::Error unlockErr;
			m_registeredFileLock->unlock(unlockErr);
			delete m_registeredFileLock;
			m_registeredFileLock = nullptr;

			if (err != Internal::LockedFileAccessor::Error::none ||
				unlockErr != Internal::FileLock::Error::none)
			{
				return false;
			}
			return true;
		}
		int JDUserRegistration::unregisterInactiveUsers_internal(Internal::LockedFileAccessor& registerFile) const
		{
			std::vector<JDUser> users;
#ifdef JD_USE_QJSON
			std::vector<QJsonObject> userJsons;
#else
			JsonArray userJsons;
#endif

			auto err = registerFile.readJsonFile(userJsons);
			if (err != Internal::LockedFileAccessor::Error::none)
			{
				return 0;
			}
			int removed = 0;
			for (auto& userJson : userJsons)
			{
				JDUser user;
				if (user.load(userJson))
				{
					if (!isSessionIDActive(user.getSessionID()))
					{
						removed += (tryToDeleteSessionFile(user.getSessionID()) == true);
					}
					else
						users.push_back(user);
				}
			}
			err = registerFile.writeJsonFile(userJsons);

			std::vector<std::string> lockFiles = Internal::FileLock::getLockFileNamesInDirectory(getPath());
			for (size_t i = 0; i < lockFiles.size(); ++i)
			{
				bool found = false;
				for (auto& user : users)
				{
					if (user.getSessionID() == lockFiles[i])
						found = true;
				}
				if (!found)
				{
					removed += (tryToDeleteSessionFile(lockFiles[i]) == true);
				}
			}

			return removed;
		}

		bool JDUserRegistration::isSessionIDActive(const std::string& sessionID) const
		{
			return Internal::FileLock::isLockInUse(getPath(), sessionID);
		}
		bool JDUserRegistration::tryToDeleteSessionFile(const std::string& sessionID) const
		{
			return Internal::FileLock::deleteFile(getPath(), sessionID);
		}

		std::string JDUserRegistration::getPath() const
		{
			return m_manager.getDatabasePath() + "\\" + m_registrationSubFolder;
		}
		std::string JDUserRegistration::getRegisterFilePath() const
		{
			return getPath() +"\\"+m_registrationFileName + ".json";
		}


#ifdef JD_USE_QJSON
		bool JDUserRegistration::load(const QJsonObject& obj)
#else
		bool JDUserRegistration::load(const JsonObject& obj)
#endif
		{
			JD_UNUSED(obj);
			return false;
		}


#ifdef JD_USE_QJSON
		bool JDUserRegistration::save(QJsonObject& obj) const
#else
		bool JDUserRegistration::save(JsonObject& obj) const
#endif
		{
			JD_UNUSED(obj);
			return false;
		}
	}
}