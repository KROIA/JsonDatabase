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

		//static void deserializeValue_internal(const std::string& json, size_t& index, JsonValue& out);
		static void deserializeValue_internal(const std::string& json, size_t& index, JsonValue& out, Internal::WorkProgress* progress);
		//static void deserializeValueSplitted_internal(const std::string& json, size_t& index, JsonValue& out);
		static void deserializeValueSplitted_internal(const std::string& json, size_t& index, JsonValue& out, Internal::WorkProgress* progress);
		//static void deserializeObject_internal(const std::string& json, size_t& index, JsonValue& out);
		static void deserializeObject_internal(const std::string& json, size_t& index, JsonValue& out, Internal::WorkProgress* progress);
		//static void deserializeArray_internal(const std::string& json, size_t& index, JsonValue& out);
		static void deserializeArray_internal(const std::string& json, size_t& index, JsonValue& out, Internal::WorkProgress* progress);
		//static void deserializeArraySplitted_internal(const std::string& json, size_t& index, JsonValue& out);
		static void deserializeArraySplitted_internal(const std::string& json, size_t& index, JsonValue& out, Internal::WorkProgress* progress);

		//static void deserializePair(const std::string& json, size_t& index, std::pair<std::string, JsonValue> &pairOut);
		static void deserializePair(const std::string& json, size_t& index, std::pair<std::string, JsonValue> &pairOut, Internal::WorkProgress* progress);
		static void deserializeString(const std::string& json, size_t& index, std::string &strOut);
		static double deserializeNumber(const std::string& json, size_t& index, int &intValue, bool &isInt);
		
		static bool deserializeBool(const std::string& json, size_t& index);

		static void skipWhiteSpace(const std::string& jsonString, size_t& index);
		
		static void removeChars(const std::string& jsonString, std::string& jsonStringOut,
			const std::string &removingChars);

		static void removeSpecificChars(const std::string& jsonString, std::string& jsonStringOut);

		static std::string unescapeString(const std::string &str);


		struct ArrayObjectRange
		{
			size_t start;
			size_t end;
		};
		static void findArrayObjectRange(const std::string& json, size_t index, std::vector<ArrayObjectRange> &rangeList);
	};

}