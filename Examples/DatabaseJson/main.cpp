#include <QCoreApplication>
#include <iostream>
#include "JsonDatabase.h"
#include <vector>


using namespace std;
using namespace JsonDatabase;


std::string fileData;

bool test_json_stringParse();
bool test_json_objectNesting();
bool test_json_deserialize();

int main(int argc, char* argv[])
{
	QCoreApplication a(argc, argv);

	bool success = true;
	success &= test_json_stringParse();

	success &= test_json_objectNesting();
	success &= test_json_deserialize();
	std::cout << "All tests " << (success ? "passed" : "failed") << "\n\n\n";
	return a.exec();
}


bool test_json_stringParse_str_to_number(int shuldBeType, const std::string& str)
{
	int intValue = 0;
	double doubleValue = 0;
	size_t index = 0; 
	int type = JsonDatabase::JsonDeserializer::deserializeNumber(str, intValue, doubleValue, index);
	std::string typeStr;
	switch (type)
	{
	case 0:
		typeStr = "None   ";
		break;
	case 1:
		typeStr = "Int    ";
		break;
	case 2:
		typeStr = "Double ";
		break;
	}

	std::string status = "fail ";
	if (shuldBeType == type)
	{
		status = "pass ";
	}
	std::cout << "str_to_number "<< status << typeStr << "\"" << str << "\"" << " intVal:" << intValue << " doubleVal: " << doubleValue << "\n";
	return shuldBeType == type;
}
bool test_json_stringParse_double_to_str(const std::string& shuldBeType, double val)
{
	std::string converted = JsonDatabase::JsonSerializer::serializeDouble(val);
	std::string status = "fail ";
	if(converted == shuldBeType)
	{
		status = "pass ";
	}
	std::cout << "double_to_str "<<status << val << " : " << shuldBeType << " == " << converted << "\n";
	return converted == shuldBeType;
}
bool test_json_stringParse_int_to_str(const std::string& shuldBeType, int val)
{
	std::string converted = JsonDatabase::JsonSerializer::serializeInt(val);
	std::string status = "fail ";
	if (converted == shuldBeType)
	{
		status = "pass ";
	}
	std::cout << "int_to_str "<< status << val << " : " << shuldBeType << " == " << converted << "\n";
	return converted == shuldBeType;
}
bool test_json_stringParse()
{
	std::cout << "test_json_stringParse" << " Start\n";
	bool success = true;
	enum ValueType
	{
		None = 0,
		Int = 1,
		Double = 2
	};

	std::vector< std::pair<int, std::string>> strToNumberTests = {
		{ Int, "0" },
		{ Int, "1" },
		{ Int, "12" },
		{ Int, "123" },
		{ Int, "1235" },
		{ Int, "-1235" },
		{ Int, "-1" },
		{ Int, "-0" },
		{ Double, "0.0" },
		{ Double, "1.0" },
		{ Double, "-65.2" },
		{ Double, "-6562.0" },
		{ Double, "6562.0" },
		{ Double, "6562.000" },
		{ Double, "6562.00000000000" },
		{ Double, "-6." },
		{ Double, "-6.0." },
		{ Double, "-6.0.0" },
		{ Int, "-"},
		{ Double, "-0.0"},  // Invalid double format
		{ Double, "12.34"}, // Valid double
		{ Int, "123a"},  // Invalid string for conversion
		{ None, ""},      // Empty string
		{ None, " "},     // Whitespace
		{ Int, "0x64"},  // Hexadecimal
		{ Double, "1.23e5"}, // Scientific notation
		{ Double, "1.23e-5"} // Scientific notation
	};
	for (auto& test : strToNumberTests)
	{
		success &= test_json_stringParse_str_to_number(test.first, test.second);
	}

	std::vector< std::pair<std::string, double>> doubleToStrTests = {
		{ "0.0", 0.0 },
		{ "1.0", 1.0 },
		{ "-65.25", -65.25 },
		{ "-6562.0", -6562.0 },
		{ "6562.0", 6562.0 },
		{ "6562.0", 6562.000 },
		{ "6562.0", 6562.00000000000 },
		{ "-6.0", -6.0 },
		{ "0.0", 0.0 },
		{ "-0.0", -0.0 },  // Invalid double format
		{ "12.125", 12.125 }, // Valid double
	};
	for (auto& test : doubleToStrTests)
	{
		success &= test_json_stringParse_double_to_str(test.first, test.second);
	}

	std::vector< std::pair<std::string, int>> intToStrTests = {
		{ "0", 0 },
		{ "1", 1 },
		{ "-65", -65 },
		{ "-6562", -6562 },
		{ "6563", 6563 },
		{ "6562", 6562 },
		{ "-6", -6 },
		{ "0", 0 },
		{ "0", -0 },  
		{ "12", 12 }, 
		{ "123", 123 },
		{ "-123", -123 }
	};
	for (auto& test : intToStrTests)
	{
		success &= test_json_stringParse_int_to_str(test.first, test.second);
	}
	
	std::cout << "test_json_stringParse" << " End " << (success ? "passed" : "failed") << "\n\n\n";
	return success;
}


bool test_json_objectNesting()
{
	std::cout << "test_json_objectNesting" << " Start\n";
	bool success = true;
	JsonValue stringValue("stringValue");
	JsonValue intValue(1);
	JsonValue doubleValue(1.0);
	JsonValue boolValue(true);
	JsonValue nullValue;

	JsonArray jsonArray({ stringValue, intValue, doubleValue, boolValue, nullValue });
	JsonValue arrayValue(jsonArray);

	JsonObject jsonObject(
		{ 
			{ "string1", "1This is a test text\nentered \"new line\": \"1\"" },
			{ "string2", "2This is a test text\nentered \"new line\": \"1\"" },
			{ "string3", "3This is a test text\nentered \"new line\": \"1\"" },
			{ "int", intValue }, 
			{ "double", doubleValue }, 
			{ "bool", boolValue }, 
			{ "null", nullValue }, 
			{ "array", arrayValue } 
		});


	JsonArray jsonArray2({ stringValue, intValue, doubleValue, boolValue, nullValue, arrayValue, jsonObject });
	JsonArray jsonArray3({ jsonObject, jsonObject, jsonObject, jsonObject, jsonArray });

	cout << "\n\nstringValue:  \n" << stringValue << endl;
	cout << "\n\nintValue:  \n" << intValue << endl;
	cout << "\n\ndoubleValue:  \n" << doubleValue << endl;
	cout << "\n\nboolValue: \n" << boolValue << endl;
	cout << "\n\nnullValue: \n" << nullValue << endl;
	cout << "\n\narrayValue: \n" << arrayValue << endl;
	cout << "\n\njsonObject: \n" << jsonObject << endl;
	cout << "\n\njsonArray2: \n" << jsonArray2 << endl;

	fileData = JsonValue(jsonArray3).toString();

	cout << "\n\njsonArray3: \n" << fileData << endl;


	


	std::cout << "test_json_objectNesting" << " End " << (success ? "passed" : "failed") << "\n\n\n";
	return success;
}

bool test_json_deserialize()
{
	std::cout << "test_json_deserialize" << " Start\n";
	bool success = true;

	std::string input = "This is a test text\nentered new line: \"1\"  \"This is a test text\n entered newline. same but as comment\"";
	std::string output;
	JsonDeserializer::nornmalizeJsonString(input, output);

	cout << "\n\nstandard string     : \n\"" << input << "\"" << endl;
	cout << "\n\nnornmalizeJsonString: \n\"" << output << "\"" << endl;

	JsonDeserializer deserializer;
	JsonValue jsonObject = deserializer.deserializeArray(fileData);

	cout << "\n\njsonObject: \n" << jsonObject << endl;

	

	std::cout << "test_json_deserialize" << " End " << (success ? "passed" : "failed") << "\n\n\n";
	return success;
}