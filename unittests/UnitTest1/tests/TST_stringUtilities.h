#pragma once

#include "UnitTest.h"
#include <QObject>
#include <QCoreapplication>


#include "JsonDatabase.h"
#include "utilities/StringUtilities.h"


class TST_stringUtilities : public UnitTest::Test
{
	TEST_CLASS(TST_stringUtilities)
public:
	TST_stringUtilities()
		: Test("TST_stringUtilities")
	{
		ADD_TEST(TST_stringUtilities::dateTime);
		ADD_TEST(TST_stringUtilities::test2);

	}

private:

	// Tests
	TEST_FUNCTION(dateTime)
	{
		TEST_START;
		std::string dateStr = "01.12.2021";
		std::string timeStr = "12:34:56:123";

		QDate date = Utilities::stringToQDate(dateStr);
		TEST_ASSERT(date.year() == 2021);
		TEST_ASSERT(date.month() == 12);
		TEST_ASSERT(date.day() == 1);

		QTime time = Utilities::stringToQTime(timeStr);
		TEST_ASSERT(time.hour() == 12);
		TEST_ASSERT(time.minute() == 34);
		TEST_ASSERT(time.second() == 56);
		TEST_ASSERT(time.msec() == 123);



		date = Utilities::fastStringToQDate(dateStr);

		TEST_ASSERT(date.year() == 2021);
		TEST_ASSERT(date.month() == 12);
		TEST_ASSERT(date.day() == 1);

		time = Utilities::fastStringToQTime(timeStr);

		TEST_ASSERT(time.hour() == 12);
		TEST_ASSERT(time.minute() == 34);
		TEST_ASSERT(time.second() == 56);
		TEST_ASSERT(time.msec() == 123);
	}




	TEST_FUNCTION(test2)
	{
		TEST_START;

		int a = 0;
		TEST_ASSERT_M(a == 0, "is a == 0?");

		int b = 0;
		if (b != 0)
		{
			TEST_FAIL("b is not 0");
		}

		// fails if a != b
		TEST_COMPARE(a, b);
	}

};
