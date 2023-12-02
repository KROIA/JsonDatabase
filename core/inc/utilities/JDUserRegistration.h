#pragma once

#include "JD_base.h"
#include "JDDeclaration.h"
#include "JDSerializable.h"
#include "JDUser.h"

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

			bool registerUser(const JDUser& user);
			bool unregisterUser(const JDUser& user);

			bool isUserRegistered(const JDUser& user) const;
			bool isUserRegistered(const std::string& sessionID) const;

			std::vector<JDUser> getRegisteredUsers() const;
			int unregisterInactiveUsers();

#ifdef JD_USE_QJSON
			bool load(const QJsonObject& obj) override;
			bool save(QJsonObject& obj) const override;
#else
			bool load(const JsonObject& obj) override;
			bool save(JsonObject& obj) const override;
#endif
		private:
			JDManager &m_manager;

		};
	}
}