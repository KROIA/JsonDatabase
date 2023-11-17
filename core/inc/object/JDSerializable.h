#pragma once

#include "JD_base.h"
#include "JDDeclaration.h"
#include "object/JDObjectID.h"

#ifdef JD_USE_QJSON
#include <QJsonObject>
#else
#include "Json/JsonValue.h"
#endif



namespace JsonDatabase
{
#ifdef JD_USE_QJSON
	using JsonObject = QJsonObject;
#endif

	class JSONDATABASE_EXPORT JDSerializable
	{
	public:
		virtual ~JDSerializable() {}

		virtual bool load(const JsonObject& obj) = 0;
		virtual bool save(JsonObject& obj) const = 0;

#ifdef JD_USE_QJSON
		//virtual bool load(const QJsonObject& obj) = 0;
		//virtual bool save(QJsonObject& obj) const = 0;

		static bool getJsonValue(const QJsonObject& obj, QJsonObject& value, const QString& key);
		static bool getJsonValue(const QJsonObject& obj, QVariant& value, const QString& key);
		static bool getJsonValue(const QJsonObject& obj, QString& value, const QString& key);
		static bool getJsonValue(const QJsonObject& obj, std::string& value, const QString& key);
		static bool getJsonValue(const QJsonObject& obj, int& value, const QString& key);
		static bool getJsonValue(const QJsonObject& obj, JDObjectID& value, const QString& key);
		static bool getJsonValue(const QJsonObject& obj, double& value, const QString& key);
		static bool getJsonValue(const QJsonObject& obj, float& value, const QString& key);
		static bool getJsonValue(const QJsonObject& obj, bool& value, const QString& key);
#else
		


		static bool getJsonValue(const JsonObject& obj, JsonObject& value, const std::string& key);
		//static bool getJsonValue(const JsonObject& obj, QVariant& value, const std::string& key);
		static bool getJsonValue(const JsonObject& obj, QString& value, const std::string& key);
		static bool getJsonValue(const JsonObject& obj, std::string& value, const std::string& key);
		static bool getJsonValue(const JsonObject& obj, int& value, const std::string& key);
		static bool getJsonValue(const JsonObject& obj, JDObjectID& value, const std::string& key);
		static bool getJsonValue(const JsonObject& obj, double& value, const std::string& key);
		static bool getJsonValue(const JsonObject& obj, float& value, const std::string& key);
		static bool getJsonValue(const JsonObject& obj, bool& value, const std::string& key);
#endif
	protected:

	};
}
