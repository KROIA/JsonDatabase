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
	class JSONDATABASE_EXPORT JDSerializable
	{
	public:
		virtual ~JDSerializable() {}
#ifdef JD_USE_QJSON
		virtual bool load(const QJsonObject& obj) = 0;
		virtual bool save(QJsonObject& obj) const = 0;

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
		virtual bool load(const JsonObject& obj) = 0;
		virtual bool save(JsonObject& obj) const = 0;
#endif
	protected:

	};
}
