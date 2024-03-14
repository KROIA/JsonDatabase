#pragma once
#include "JsonDatabase_base.h"

#include "JsonValue.h"
#include "manager/async/WorkProgress.h"

namespace JsonDatabase
{
	class JSON_DATABASE_EXPORT_EXPORT JsonSerializer
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



		static std::string serializeLong(long value);
		static std::string serializeDouble(double value);
		static std::string serializeBool(bool value);

		static void serializeLong(long value, std::string& serializedOut);
		static void serializeDouble(double value, std::string& serializedOut);
		static void serializeBool(bool value, std::string& serializedOut);


	private:


		void serializeValue(const JsonValue& value, std::string& serializedOut, int &indent);
		void serializeObject(const JsonObject& object, std::string& serializedOut, int& indent);
		void serializeArray(const JsonArray& array, std::string& serializedOut, int &indent);
		void serializeArray(const JsonArray& array, std::string& serializedOut, int &indent, Internal::WorkProgress* progress);




		static std::string serializeString(const std::string& str);
		static void serializeString(const std::string& str, std::string& serializedOut);
		static const std::string &serializeNull();

		static void escapeString(const std::string& str, std::string& serializedOut);


		bool m_useTabs = true;
		int m_tabSize = 4;

		bool m_useNewLinesInObjects = true;
		bool m_useNewLineAfterObject = true;
		bool m_useSpaces = true;

		char m_indentChar = ' ';

	};

}
