#pragma once

#include "JsonDatabase_base.h"
#include "JsonDatabase_Declaration.h"
#include "JDUser.h"
#include "utilities/filesystem/AbstractRegistry.h"

#include <json/JsonValue.h>


/*
	needs the path to the database
	when the paths changes, it needs a update to the new path and log the user out 

*/
namespace JsonDatabase
{
	namespace Utilities
	{
		class JSON_DATABASE_EXPORT_EXPORT JDUserRegistration : public AbstractRegistry
		{
		public:

			JDUserRegistration();
			~JDUserRegistration();

			bool registerUser(const JDUser& user, std::string &generatedSessionIDOut);
			bool unregisterUser();
			
			bool isUserRegistered() const;
			bool isUserRegistered(const JDUser& user) const;
			bool isUserRegistered(const std::string& sessionID) const;

			std::vector<JDUser> getRegisteredUsers() const;
			int unregisterInactiveUsers() const;

		private:
			class JSON_DATABASE_EXPORT_EXPORT LockEntryObjectImpl : public LockEntryObject
			{
			public:
				LockEntryObjectImpl(const std::string& key);
				LockEntryObjectImpl(const std::string& key, const JDUser &user);
				~LockEntryObjectImpl();


				void setUser(const JDUser& user);
				const JDUser& getUser() const;


				bool load(const JsonObject& obj) override;
				bool save(JsonObject& obj) const override;

				struct JsonKeys
				{
					static const std::string user;
				};
			private:
				JDUser m_user;
			};


			void onCreateFiles() override;
			void onDatabasePathChangeStart(const std::string& newPath) override;
			void onDatabasePathChangeEnd() override;
			void onNameChange(const std::string& newName) override;


			std::shared_ptr<LockEntryObjectImpl> m_registeredUser;
			std::string m_registeredSessionID;
			bool m_isRegistered;
			unsigned int m_registryOpenTimeoutMs;
		};
	}
}