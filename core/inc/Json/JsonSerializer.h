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

		void serializeValue(const JsonValue& value, std::string &serializedOut);
		void serializeObject(const JsonObject& object, std::string& serializedOut);
		void serializeArray(const JsonArray& array, std::string& serializedOut);

		static std::string serializeInt(int value);
		static std::string serializeDouble(double value);
		static std::string serializeBool(bool value);

		static void serializeInt(int value, std::string& serializedOut);
		static void serializeDouble(double value, std::string& serializedOut);
		static void serializeBool(bool value, std::string& serializedOut);
	private:
		void serializeValue(const JsonValue& value, std::string& serializedOut, int &indent);
		void serializeObject(const JsonObject& object, std::string& serializedOut, int& indent);
		void serializeArray(const JsonArray& array, std::string& serializedOut, int &indent);




		static std::string serializeString(const std::string& str);
		static void serializeString(const std::string& str, std::string& serializedOut);
		static const std::string &serializeNull();

		static void escapeString(const std::string& str, std::string& serializedOut);


		//int m_indent = 0;
		bool m_useTabs = false;
		int m_tabSize = 4;

		bool m_useNewLinesInObjects = false;
		bool m_useNewLineAfterObject = true;
		bool m_useSpaces = false;

		char m_indentChar = ' ';
	};

}