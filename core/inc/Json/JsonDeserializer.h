#pragma once
#include "JsonDatabase_base.h"


#include "JsonValue.h"

#include "manager/async/WorkProgress.h"

namespace JsonDatabase
{
	class JSON_DATABASE_EXPORT JsonDeserializer
	{
	public:

		
		JsonValue deserializeValue(const std::string& json);
		JsonObject deserializeObject(const std::string& json);
		JsonArray deserializeArray(const std::string& json);
		JsonValue deserializeValue(const std::string& json, Internal::WorkProgress* progress);
		JsonObject deserializeObject(const std::string& json, Internal::WorkProgress* progress);
		JsonArray deserializeArray(const std::string& json, Internal::WorkProgress* progress);

		bool deserializeValue(const std::string& json, JsonValue& valueOut);
		bool deserializeObject(const std::string& json, JsonObject& valueOut);
		bool deserializeArray(const std::string& json, JsonArray& valueOut);
		bool deserializeValue(const std::string& json, JsonValue& valueOut, Internal::WorkProgress* progress);
		bool deserializeObject(const std::string& json, JsonObject& valueOut, Internal::WorkProgress* progress);
		bool deserializeArray(const std::string& json, JsonArray& valueOut, Internal::WorkProgress* progress);

		static void nornmalizeJsonString(const std::string& jsonString, std::string& jsonStringOut);
		static int deserializeNumber(const std::string& jsonString, long& longValue, double& doubleValue);

		static std::string unescapeString(const std::string& str);
	private:
		struct Buffer
		{
			Buffer(const std::string & str) 
				: m_current(&str[0])
				, m_start(m_current)
				, m_size(str.size())
				, m_end(m_current + m_size)
			{ }

			Buffer()
				: m_current(nullptr)
				, m_start(nullptr)
				, m_size(0)
				, m_end(nullptr)
			{ }

			inline char peek() const
			{
				if(m_current >= m_end || !m_current)
					return 0;
				return *m_current;
			}
			inline char next()
			{
				char c = peek();
				m_current++;
				return c;
			}
			inline bool hasNext() const
			{
				return m_current < m_end;
			}
			inline void skip(size_t count = 1)
			{
				m_current += count;
			}
			inline void skipBack(size_t count = 1)
			{
				m_current -= count;
			}
			inline const char* getCurrent() const
			{
				if(m_current >= m_end)
					return nullptr;
				return m_current;
			}
			inline void setCurrent(const char* newCurrent)
			{
				if (newCurrent >= m_end || newCurrent < m_start)
				{
					m_current = nullptr;
					return;
				}
				m_current = newCurrent;
			}
			inline size_t getIndex() const
			{
				if(m_current >= m_end)
					return m_size;
				return m_current - m_start;
			}
			inline size_t getRemainingSize() const
			{
				if(m_current >= m_end)
					return 0;
				return m_end - m_current;
			}
			inline size_t size() const
			{
				return m_size;
			}
			inline const char* start() const
			{
				return m_start;
			}
			inline const char* end() const
			{
				return m_end;
			}
			inline void reset()
			{
				m_current = m_start;
			}
			inline void reset(const std::string& str)
			{
				m_current = &str[0];
				m_start = m_current;
				m_size = str.size();
				m_end = m_current + m_size;
			}
			inline void reset(const char* str, size_t strSize)
			{
				m_current = str;
				m_start = m_current;
				m_size = strSize;
				m_end = m_current + m_size;
			}
			inline void setString(const std::string& str)
			{
				m_current = &str[0];
				m_start = m_current;
				m_size = str.size();
				m_end = m_current + m_size;
			}
			inline void setString(const char* str, size_t strSize)
			{
				m_current = str;
				m_start = m_current;
				m_size = strSize;
				m_end = m_current + m_size;
			}
		private:
			const char* m_current;
			const char* m_start;
			size_t m_size;
			const char* m_end;
		};


		/*
			Returns 0 if the string is not a number
			Returns 1 if the string is an integer
			Returns 2 if the string is a double
		*/
		static int deserializeNumber(Buffer& json, long& longValue, double& doubleValue);
		

		static bool deserializeValue_internal(Buffer& json, JsonValue& out);
		static bool deserializeValue_internal(Buffer& json, JsonValue& out, Internal::WorkProgress* progress);
		static bool deserializeValueSplitted_internal(Buffer& json, JsonValue& out);
		static bool deserializeValueSplitted_internal(Buffer& json, JsonValue& out, Internal::WorkProgress* progress);
		static bool deserializeObject_internal(Buffer& json, JsonObject& out);
		static bool deserializeObject_internal(Buffer& json, JsonObject& out, Internal::WorkProgress* progress);
		//static void deserializeObjectSplitted_internal(Buffer& json, JsonObject& out);
		//static void deserializeObjectSplitted_internal(Buffer& json, JsonObject& out, Internal::WorkProgress* progress);
		static bool deserializeArray_internal(Buffer& json, JsonArray& out);
		static bool deserializeArray_internal(Buffer& json, JsonArray& out, Internal::WorkProgress* progress);

		static bool deserializeArraySplitted_internal(Buffer& json, JsonArray& out, Internal::WorkProgress* progress);

		static bool deserializePair(Buffer& json, std::pair<std::string, JsonValue> &pairOut);
		static bool deserializePair(Buffer& json, std::pair<std::string, JsonValue> &pairOut, Internal::WorkProgress* progress);
		static bool deserializeString(Buffer& json, std::string &strOut);
		//static void deserializeNumber(Buffer& json, double &doubleValue, int &intValue, bool &isInt);
		
		static bool deserializeBool(Buffer& json, bool &valueOut);

		static void skipWhiteSpace(const std::string& jsonString, size_t& index);
		
		static void removeChars(const std::string& jsonString, std::string& jsonStringOut,
			const std::string &removingChars);

		static void removeSpecificChars(const std::string& jsonString, std::string& jsonStringOut);
		static void removeSpecificChars(const char* jsonString, char* jsonStringOut, size_t size);
		
		static const char* findFirstNotOfNumberStr(const char* str);


		struct ArrayObjectRange
		{
			size_t start;
			size_t end;
		};
		static void findArrayObjectRange(Buffer& json, std::vector<ArrayObjectRange>& rangeList);
	};

}
