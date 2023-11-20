#include <QCoreApplication>
#include <iostream>
#include "JsonDatabase.h"
#include <vector>
#include <QFile>

using namespace std;
using namespace JsonDatabase;


std::string fileData;

bool test_json_stringParse();
bool test_json_objectNesting();
bool test_json_deserialize();
bool test_stingNormalization();

int main(int argc, char* argv[])
{
	QCoreApplication a(argc, argv);
	Profiler::startProfiler();
	bool success = true;
	//success &= test_json_stringParse();

	//success &= test_json_objectNesting();
	//success &= test_json_deserialize();
	success &= test_stingNormalization();
	std::cout << "All tests " << (success ? "passed" : "failed") << "\n\n\n";
	Profiler::stopProfiler("json.prof");
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

bool test_stingNormalization()
{
	std::cout << "test_stingNormalization" << " Start\n";
	bool success = true;

	QFile file("asyncDatabase/Person.json");
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		std::cout << "test_stingNormalization" << " End " << (success ? "passed" : "failed") << "\n\n\n";
		return false;
	}
	std::string fileData = file.readAll().toStdString();
	file.close();

	struct PerformanceData
	{
		double time;
		clock_t clocks;
	};
	std::vector<PerformanceData> performanceData;

	size_t length = fileData.length();
	size_t iterations = 100;
	
	double time = 0;
	clock_t clocks = 0;
	performanceData.reserve(iterations);
	for (size_t i = 0; i < iterations; ++i)
	{
		std::string output;
		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
		clock_t startClock = clock();

		JsonDeserializer::nornmalizeJsonString(fileData, output);

		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		clock_t endClock = clock();
		PerformanceData data;
		data.time = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000.0;
		data.clocks = endClock - startClock;
		
		
		
		double processedCharsPerSecond = length / (data.time / 1000.0);
		double clocksPerChar = data.clocks / (double)length;
		std::cout << "[" << i<<"] process time: " << data.time << "ms clocks: " << data.clocks
			<< " chars/sec: " << processedCharsPerSecond << " clk/char: " << clocksPerChar << endl;
		time += data.time;
		clocks += data.clocks;
		performanceData.push_back(data);
	}
	time /= iterations;
	clocks /= iterations;

	double processedCharsPerSecond = length / (time / 1000.0);
	double clocksPerChar = clocks / (double)length;
	std::cout << "process time: "<< time << "ms clocks: "<< clocks 
		<< " chars/sec: "<< processedCharsPerSecond << " clk/char: "<< clocksPerChar << endl;

	std::cout << "test_stingNormalization" << " End " << (success ? "passed" : "failed") << "\n\n\n";

	// Save performance data to csv file using QFile and ";" as separator
	QFile csvFile("performanceData.csv");
	if (!csvFile.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		std::cout << "test_stingNormalization" << " End " << (success ? "passed" : "failed") << "\n\n\n";
		return false;
	}
	QTextStream csvStream(&csvFile);
	csvStream << "time;clocks\n";
	for (auto& data : performanceData)
	{
		csvStream << data.time << ";" << data.clocks << "\n";
	}
	csvFile.close();
	
	return success;
}



