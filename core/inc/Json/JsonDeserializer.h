#pragma once
#include "JD_base.h"

#if JD_ACTIVE_JSON == JD_JSON_QT

#elif JD_ACTIVE_JSON == JD_JSON_GLAZE || JD_ACTIVE_JSON == JD_JSON_INTERNAL
#include "JsonValue.h"

#include "manager/async/WorkProgress.h"

namespace JsonDatabase
{
	class JSONDATABASE_EXPORT JsonDeserializer
	{
	public:

		//JsonValue deserialize(const std::string& jsonString);
		JsonValue deserializeValue(const std::string& json);
		JsonObject deserializeObject(const std::string& json);
		JsonArray deserializeArray(const std::string& json);
		JsonValue deserializeValue(const std::string& json, Internal::WorkProgress* progress);
		JsonObject deserializeObject(const std::string& json, Internal::WorkProgress* progress);
		JsonArray deserializeArray(const std::string& json, Internal::WorkProgress* progress);

		void deserializeValue(const std::string& json, JsonValue& valueOut);
		void deserializeObject(const std::string& json, JsonObject& valueOut);
		void deserializeArray(const std::string& json, JsonArray& valueOut);
		void deserializeValue(const std::string& json, JsonValue& valueOut, Internal::WorkProgress* progress);
		void deserializeObject(const std::string& json, JsonObject& valueOut, Internal::WorkProgress* progress);
		void deserializeArray(const std::string& json, JsonArray& valueOut, Internal::WorkProgress* progress);

		static void nornmalizeJsonString(const std::string& jsonString, std::string& jsonStringOut);
		static int deserializeNumber(const std::string& jsonString, int& intValue, double& doubleValue);

		static std::string unescapeString(const std::string& str);
	private:
#if JD_ACTIVE_JSON == JD_JSON_INTERNAL
		struct Buffer
		{
			Buffer(const std::string & str) 
				: current(&str[0])
				, start(current)
				, size(str.size())
				, end(current + size)
			{ }

			Buffer()
				: current(nullptr)
				, start(nullptr)
				, size(0)
				, end(nullptr)
			{ }

			const char* current;
			const char* start;
			size_t size;
			const char* end;
		};


		/*
			Returns 0 if the string is not a number
			Returns 1 if the string is an integer
			Returns 2 if the string is a double
		*/
		static int deserializeNumber(Buffer& json, int& intValue, double& doubleValue);
		

		static void deserializeValue_internal(Buffer& json, JsonValue& out);
		static void deserializeValue_internal(Buffer& json, JsonValue& out, Internal::WorkProgress* progress);
		static void deserializeValueSplitted_internal(Buffer& json, JsonValue& out);
		static void deserializeValueSplitted_internal(Buffer& json, JsonValue& out, Internal::WorkProgress* progress);
		static void deserializeObject_internal(Buffer& json, JsonObject& out);
		static void deserializeObject_internal(Buffer& json, JsonObject& out, Internal::WorkProgress* progress);
		//static void deserializeObjectSplitted_internal(Buffer& json, JsonObject& out);
		//static void deserializeObjectSplitted_internal(Buffer& json, JsonObject& out, Internal::WorkProgress* progress);
		static void deserializeArray_internal(Buffer& json, JsonArray& out);
		static void deserializeArray_internal(Buffer& json, JsonArray& out, Internal::WorkProgress* progress);

		static void deserializeArraySplitted_internal(Buffer& json, JsonArray& out, Internal::WorkProgress* progress);

		static void deserializePair(Buffer& json, std::pair<std::string, JsonValue> &pairOut);
		static void deserializePair(Buffer& json, std::pair<std::string, JsonValue> &pairOut, Internal::WorkProgress* progress);
		static void deserializeString(Buffer& json, std::string &strOut);
		static double deserializeNumber(Buffer& json, int &intValue, bool &isInt);
		
		static bool deserializeBool(Buffer& json);

		static void skipWhiteSpace(const std::string& jsonString, size_t& index);
		
		static void removeChars(const std::string& jsonString, std::string& jsonStringOut,
			const std::string &removingChars);

		static void removeSpecificChars(const std::string& jsonString, std::string& jsonStringOut);
		
		static const char* findFirstNotOfNumberStr(const char* str);


		struct ArrayObjectRange
		{
			size_t start;
			size_t end;
		};
		static void findArrayObjectRange(Buffer& json, std::vector<ArrayObjectRange> &rangeList);
#endif
	};

}
#endif