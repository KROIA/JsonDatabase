#pragma once

#include "JsonDatabase_base.h"
#include "JsonDatabase_Declaration.h"
#include "object/JDObjectID.h"
#include "Json/JsonValue.h"




namespace JsonDatabase
{
	namespace Utilities
	{
		class JSON_DATABASE_EXPORT_EXPORT JDSerializable
		{
		public:
			virtual ~JDSerializable() {}




			virtual bool load(const JsonObject& obj) = 0;
			virtual bool save(JsonObject& obj) const = 0;

			static bool getJsonValue(const JsonObject& obj, JsonObject& value, const std::string& key);
			//static bool getJsonValue(const JsonObject& obj, QVariant& value, const std::string& key);
			static bool getJsonValue(const JsonObject& obj, QString& value, const std::string& key);
			static bool getJsonValue(const JsonObject& obj, std::string& value, const std::string& key);
			static bool getJsonValue(const JsonObject& obj, long& value, const std::string& key);
			//static bool getJsonValue(const JsonObject& obj, JDObjectID::IDType& value, const std::string& key);
			static bool getJsonValue(const JsonObject& obj, double& value, const std::string& key);
			static bool getJsonValue(const JsonObject& obj, float& value, const std::string& key);
			static bool getJsonValue(const JsonObject& obj, bool& value, const std::string& key);

		protected:

		};
	}
}
