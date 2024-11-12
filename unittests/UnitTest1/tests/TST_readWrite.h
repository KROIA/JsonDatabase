#pragma once

#include "UnitTest.h"
#include <QObject>
#include <QCoreapplication>

#include "Person.h"





class TST_readWrite : public UnitTest::Test
{
	TEST_CLASS(TST_readWrite)
public:
	TST_readWrite()
		: Test("TST_readWrite")
	{
		ADD_TEST(TST_readWrite::initialize);
		ADD_TEST(TST_readWrite::test2);

	}

private:

	// Tests
	TEST_FUNCTION(initialize)
	{
		TEST_START;


		int a = 0;
		TEST_MESSAGE("is a == 0?");
		TEST_ASSERT(a == 0);
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
