#pragma once

#include "JD_base.h"
#include "JDDeclaration.h"
#include "object/JDObjectID.h"
#include "Json/JsonValue.h"




namespace JsonDatabase
{
	namespace Utilities
	{
		class JSONDATABASE_EXPORT JDSerializable
		{
		public:
			virtual ~JDSerializable() {}




			virtual bool load(const JsonObject& obj) = 0;
			virtual bool save(JsonObject& obj) const = 0;

			static bool getJsonValue(const JsonObject& obj, JsonObject& value, const std::string& key);
			//static bool getJsonValue(const JsonObject& obj, QVariant& value, const std::string& key);
			static bool getJsonValue(const JsonObject& obj, QString& value, const std::string& key);
			static bool getJsonValue(const JsonObject& obj, std::string& value, const std::string& key);
			static bool getJsonValue(const JsonObject& obj, int& value, const std::string& key);
			//static bool getJsonValue(const JsonObject& obj, JDObjectID::IDType& value, const std::string& key);
			static bool getJsonValue(const JsonObject& obj, double& value, const std::string& key);
			static bool getJsonValue(const JsonObject& obj, float& value, const std::string& key);
			static bool getJsonValue(const JsonObject& obj, bool& value, const std::string& key);

		protected:

		};
	}
}
