#pragma once

#include "JsonDatabase_base.h"
#include "JsonDatabase_Declaration.h"
#include "utilities/JDSerializable.h"
#include "changehistory/IChangeTransaction.h"

namespace JsonDatabase
{
	class JSON_DATABASE_API ChangeLogfileEntry : public Utilities::JDSerializable
	{
	public:
		ChangeLogfileEntry();
		~ChangeLogfileEntry();

		virtual const std::string className() const { return "ChangeLogfileEntry"; }


		bool load(const JsonObject& obj) override;
		bool save(JsonObject& obj) const override;


		void setChangeTransactions(const std::vector<std::shared_ptr<IChangeTransaction>>& transactions) { m_changeTransactions = transactions; }
		void addChangeTransaction(std::shared_ptr<IChangeTransaction> transaction) { m_changeTransactions.push_back(transaction); }
		void clearChangeTransactions() { m_changeTransactions.clear(); }

	protected:
		/*class JSON_DATABASE_API AutoObjectAddToRegistry
		{
		public:
			AutoObjectAddToRegistry(ChangeLogfileEntry obj);
			int addToRegistry(ChangeLogfileEntry obj);
		};
        class JSON_DATABASE_API ChangeLogFileEntryRegistry
        {
            ChangeLogFileEntryRegistry();
        public:
            enum Error
            {
                none,
                emptyClassName,
                typeAlreadyRegistered,
                objIsNullptr
            };


            static ChangeLogFileEntryRegistry& getInstance();


            static Error registerType(const ChangeLogfileEntry& obj);
            static const std::map<std::string, ChangeLogfileEntry>& getRegisteredTypes();

            static const ChangeLogfileEntry& getObjectDefinition(const JsonObject& json);
            static std::string getObjectTypeString(const JsonObject& json);

            static const ChangeLogfileEntry& getObjectDefinition(const std::string& className);

        private:
            std::map<std::string, ChangeLogfileEntry> m_registry;
        };*/


		std::vector<std::shared_ptr<IChangeTransaction>> m_changeTransactions;
	};
}