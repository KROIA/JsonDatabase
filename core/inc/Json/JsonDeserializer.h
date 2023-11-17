#pragma once
#include "JD_base.h"
#include "JsonValue.h"

namespace JsonDatabase
{
	class JSONDATABASE_EXPORT JsonDeserializer
	{
	public:

		//JsonValue deserialize(const std::string& jsonString);
		JsonValue deserializeValue(const std::string& json);
		JsonValue deserializeObject(const std::string& json);
		JsonValue deserializeArray(const std::string& json);	

		/*
			Returns 0 if the string is not a number
			Returns 1 if the string is an integer
			Returns 2 if the string is a double
		*/
		static int deserializeNumber(const std::string& str, int& intValue, double& doubleValue, size_t &index);
		static void nornmalizeJsonString(const std::string& jsonString, std::string& jsonStringOut);
	private:

		static JsonValue deserializeValue_internal(const std::string& json, size_t& index);
		static JsonValue deserializeObject_internal(const std::string& json, size_t& index);
		static JsonValue deserializeArray_internal(const std::string& json, size_t& index);

		static std::pair<std::string, JsonValue> deserializePair(const std::string& json, size_t& index);
		static std::string deserializeString(const std::string& json, size_t& index);
		//static double deserializeNumber(const std::string& json, size_t& index, int &intValue, bool &isInt);
		
		static bool deserializeBool(const std::string& json, size_t& index);

		static void skipWhiteSpace(const std::string& jsonString, size_t& index);
		
		static void removeChars(const std::string& jsonString, std::string& jsonStringOut,
			const std::vector<char>& chars);

		static std::string unescapeString(const std::string &str);
		/*JsonValue parseValue(const std::string& jsonString);
        void skipWhiteSpace(const std::string& jsonString);

		JsonValue parseObject(const std::string& jsonString);
		JsonValue parseArray(const std::string& jsonString);
		JsonValue parseString(const std::string& jsonString);
		JsonValue parseBoolean(const std::string& jsonString);
		JsonValue parseNull(const std::string& jsonString);
		JsonValue parseInt(const std::string& jsonString);
		JsonValue parseDouble(const std::string& jsonString);*/


		//size_t position; // Helper variable to keep track of parsing position

	};

}