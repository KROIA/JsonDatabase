#pragma once
#include "JD_base.h"
#include "JsonValue.h"

#include "manager/async/WorkProgress.h"

namespace JsonDatabase
{
	class JSONDATABASE_EXPORT JsonDeserializer
	{
	public:

		//JsonValue deserialize(const std::string& jsonString);
		JsonValue deserializeValue(const std::string& json);
		JsonValue deserializeObject(const std::string& json);
		JsonValue deserializeArray(const std::string& json);
		JsonValue deserializeValue(const std::string& json, Internal::WorkProgress* progress);
		JsonValue deserializeObject(const std::string& json, Internal::WorkProgress* progress);
		JsonValue deserializeArray(const std::string& json, Internal::WorkProgress* progress);

		void deserializeValue(const std::string& json, JsonValue& valueOut);
		void deserializeObject(const std::string& json, JsonValue& valueOut);
		void deserializeArray(const std::string& json, JsonValue& valueOut);
		void deserializeValue(const std::string& json, JsonValue& valueOut, Internal::WorkProgress* progress);
		void deserializeObject(const std::string& json, JsonValue& valueOut, Internal::WorkProgress* progress);
		void deserializeArray(const std::string& json, JsonValue& valueOut, Internal::WorkProgress* progress);

		/*
			Returns 0 if the string is not a number
			Returns 1 if the string is an integer
			Returns 2 if the string is a double
		*/
		static int deserializeNumber(const std::string& str, int& intValue, double& doubleValue, size_t &index);
		static void nornmalizeJsonString(const std::string& jsonString, std::string& jsonStringOut);
	private:

		//static void deserializeValue_internal(const std::string& json, size_t& index, JsonValue &out);
		//static void deserializeObject_internal(const std::string& json, size_t& index, JsonValue& out);
		//static void deserializeArray_internal(const std::string& json, size_t& index, JsonValue& out);
		static void deserializeValue_internal(const std::string& json, size_t& index, JsonValue& out, Internal::WorkProgress* progress);
		static void deserializeObject_internal(const std::string& json, size_t& index, JsonValue& out, Internal::WorkProgress* progress);
		static void deserializeArray_internal(const std::string& json, size_t& index, JsonValue& out, Internal::WorkProgress* progress);

		//static void deserializePair(const std::string& json, size_t& index, std::pair<std::string, JsonValue> &pairOut);
		static void deserializePair(const std::string& json, size_t& index, std::pair<std::string, JsonValue> &pairOut, Internal::WorkProgress* progress);
		static void deserializeString(const std::string& json, size_t& index, std::string &strOut);
		//static void deserializeString(const std::string& json, size_t& index, std::string &strOut, Internal::WorkProgress* progress);
		static double deserializeNumber(const std::string& json, size_t& index, int &intValue, bool &isInt);
		
		static bool deserializeBool(const std::string& json, size_t& index);

		static void skipWhiteSpace(const std::string& jsonString, size_t& index);
		
		static void removeChars(const std::string& jsonString, std::string& jsonStringOut,
			const std::string &removingChars);

		static void removeSpecificChars(const std::string& jsonString, std::string& jsonStringOut);

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