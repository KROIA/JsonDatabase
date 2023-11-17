#pragma once
#include "JD_base.h"
#include "JsonValue.h"

namespace JsonDatabase
{
	class JSONDATABASE_EXPORT JsonSerializer
	{
	public:

		std::string serializeValue(const JsonValue& value);
		std::string serializeObject(const JsonObject& object);
		std::string serializeArray(const JsonArray& array);

		static std::string serializeInt(int value);
		static std::string serializeDouble(double value);
		static std::string serializeBool(bool value);
	private:
		std::string serializeString(const std::string& str);
		const std::string &serializeNull();

		static std::string escapeString(const std::string& str);


		int m_indent = 0;
		bool m_useNewLines = true;
		bool m_useSpaces = true;

	};

}