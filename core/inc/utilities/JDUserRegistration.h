#pragma once

#include "JD_base.h"
#include "JDDeclaration.h"
#include "JDSerializable.h"
#include "JDUser.h"
#include "utilities/filesystem/LockedFileAccessor.h"

#ifdef JD_USE_QJSON
#include <QJsonObject>
#else
#include <json/JsonValue.h>
#endif

/*
	needs the path to the database
	when the paths changes, it needs a update to the new path and log the user out 

*/
namespace JsonDatabase
{
	namespace Utilities
	{
		class JSONDATABASE_EXPORT JDUserRegistration : public JDSerializable
		{
		public:

			JDUserRegistration(JDManager& manager);
			~JDUserRegistration();

			bool setup();

			bool registerUser(const JDUser& user, std::string &generatedSessionIDOut);
			bool unregisterUser();
			
			bool isUserRegistered() const;
			bool isUserRegistered(const JDUser& user) const;
			bool isUserRegistered(const std::string& sessionID) const;

			std::vector<JDUser> getRegisteredUsers() const;
			int unregisterInactiveUsers() const;

#ifdef JD_USE_QJSON
			bool load(const QJsonObject& obj) override;
			bool save(QJsonObject& obj) const override;
#else
			bool load(const JsonObject& obj) override;
			bool save(JsonObject& obj) const override;
#endif
		private:
			bool unregisterUser_internal();
			std::vector<JDUser> getRegisteredUsers_internal(Internal::LockedFileAccessor& registerFile) const;
			int unregisterInactiveUsers_internal(Internal::LockedFileAccessor& registerFile) const;
			bool isSessionIDActive(const std::string &sessionID) const;
			bool tryToDeleteSessionFile(const std::string &sessionID) const;

			std::string getPath() const;
			std::string getRegisterFilePath() const;

			JDManager &m_manager;
			std::string m_registeredSessionID;
			Internal::FileLock *m_registeredFileLock;
			
			std::string m_registrationSubFolder;
			std::string m_registrationFileName;
			std::string m_registrationFileEnding;

		};
	}
}