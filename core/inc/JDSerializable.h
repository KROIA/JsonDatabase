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
		virtual bool load(const QJsonObject& obj) = 0;
		virtual bool save(QJsonObject& obj) const = 0;

		static bool getJsonValue(const QJsonObject& obj, QJsonObject& value, const QString& key);
		static bool getJsonValue(const QJsonObject& obj, QVariant& value, const QString& key);
		static bool getJsonValue(const QJsonObject& obj, QString& value, const QString& key);
		static bool getJsonValue(const QJsonObject& obj, std::string& value, const QString& key);
		static bool getJsonValue(const QJsonObject& obj, int& value, const QString& key);
		static bool getJsonValue(const QJsonObject& obj, double& value, const QString& key);
		static bool getJsonValue(const QJsonObject& obj, float& value, const QString& key);
		static bool getJsonValue(const QJsonObject& obj, bool& value, const QString& key);

	protected:

	};
}
