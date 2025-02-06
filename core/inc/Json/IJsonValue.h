#pragma once

#include "JsonDatabase_base.h"
#include "JsonDatabase_Declaration.h"
#include "Json/JsonValue.h"

namespace JsonDatabase
{
	class IJsonValue
	{
	public:
		virtual ~IJsonValue() {}

		virtual JsonValue toJson() const = 0;
		virtual bool fromJson(const JsonValue& value) = 0;

	};
}