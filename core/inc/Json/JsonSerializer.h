#pragma once
#include "JD_base.h"
#include "JsonValue.h"

namespace JsonDatabase
{
	class JSONDATABASE_EXPORT JsonSerializer
	{
	public:

		static std::string serializeValue(const JsonValue& value);
		static std::string serializeObject(const JsonObject& object);
		static std::string serializeArray(const JsonArray& array);
	};

}