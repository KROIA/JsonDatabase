#pragma once
#include "JD_base.h"

#if JD_ACTIVE_JSON == JD_JSON_QT

#elif JD_ACTIVE_JSON == JD_JSON_GLAZE || JD_ACTIVE_JSON == JD_JSON_INTERNAL

#include "JsonValue.h"
#include "manager/async/WorkProgress.h"

namespace JsonDatabase
{
	class JSONDATABASE_EXPORT JsonSerializer
	{
	public:

		void enableTabs(bool enable = true);
		void setTabSize(int size);
		void enableNewLinesInObjects(bool enable = true);
		void enableNewLineAfterObject(bool enable = true);
		void enableSpaces(bool enable = true);
		void setIndentChar(char indentChar);

		bool tabsEnabled() const;
		int tabSize() const;
		bool newLinesInObjectsEnabled() const;
		bool newLineAfterObjectEnabled() const;
		bool spacesEnabled() const;
		char indentChar() const;




		std::string serializeValue(const JsonValue& value);
		std::string serializeObject(const JsonObject& object);
		std::string serializeArray(const JsonArray& array);
		std::string serializeArray(const JsonArray& array, Internal::WorkProgress* progress);

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

#if JD_ACTIVE_JSON == JD_JSON_INTERNAL

		void serializeValue(const JsonValue& value, std::string& serializedOut, int &indent);
		void serializeObject(const JsonObject& object, std::string& serializedOut, int& indent);
		void serializeArray(const JsonArray& array, std::string& serializedOut, int &indent);
		void serializeArray(const JsonArray& array, std::string& serializedOut, int &indent, Internal::WorkProgress* progress);




		static std::string serializeString(const std::string& str);
		static void serializeString(const std::string& str, std::string& serializedOut);
		static const std::string &serializeNull();

		static void escapeString(const std::string& str, std::string& serializedOut);


		//int m_indent = 0;
		bool m_useTabs = true;
		int m_tabSize = 4;

		bool m_useNewLinesInObjects = true;
		bool m_useNewLineAfterObject = true;
		bool m_useSpaces = true;

		char m_indentChar = ' ';
#endif
	};

}
#endif