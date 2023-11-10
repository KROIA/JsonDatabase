#pragma once

#include "JD_base.h"
#include "JDDeclaration.h"
#include "simdjson.h"

#include <QJsonObject>

namespace JsonDatabase
{
	class JSONDATABASE_EXPORT JDSerializable
	{
	public:
		virtual ~JDSerializable() {}
		virtual bool load(const simdjson::dom::object& obj) = 0;
		virtual bool save(simdjson::dom::object& obj) const = 0;

		static bool getJsonValue(const simdjson::dom::object& obj, simdjson::dom::object& value, const std::string& key);
		// static bool getJsonValue(const simdjson::dom::object& obj, QVariant& value, const std::string& key);
		static bool getJsonValue(const simdjson::dom::object& obj, QString& value, const std::string& key);
		static bool getJsonValue(const simdjson::dom::object& obj, std::string& value, const std::string& key);
		static bool getJsonValue(const simdjson::dom::object& obj, int& value, const std::string& key);
		static bool getJsonValue(const simdjson::dom::object& obj, double& value, const std::string& key);
		static bool getJsonValue(const simdjson::dom::object& obj, float& value, const std::string& key);
		static bool getJsonValue(const simdjson::dom::object& obj, bool& value, const std::string& key);

	
	protected:

	};
}
