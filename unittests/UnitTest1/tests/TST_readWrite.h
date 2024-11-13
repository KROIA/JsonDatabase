#pragma once

#include "UnitTest.h"
#include <QObject>
#include <QCoreapplication>
#include <QDir>

#include "JsonDatabase.h"
#include "Person.h"



using namespace JsonDatabase;

class TST_readWrite : public UnitTest::Test
{
	TEST_CLASS(TST_readWrite)
public:
	TST_readWrite()
		: Test("TST_readWrite")
	{
		ADD_TEST(TST_readWrite::initialize);
		ADD_TEST(TST_readWrite::readBack);

		// Delete the Database
		QDir dbDir(dbPath.c_str());
		if (dbDir.exists())
		{
			dbDir.removeRecursively();
		}
	}

private:
	std::string dbPath = "TestDB";
	std::string dbName = "DBName";
	std::string dbUser = "User";

	std::string person0Age;
	JDObjectID::IDType person0ID;

	// Tests
	TEST_FUNCTION(initialize)
	{
		TEST_START;
		JDManager db;

		TEST_ASSERT(db.setup(dbPath, dbName, dbUser));

		// Create a Person Object
		std::vector<JDObject> persons = createPersons();

		// Should return false because the object is not in the database
		TEST_ASSERT(db.saveObject(persons[0]) == false);

		TEST_ASSERT(db.addObject(persons));

		// Should return false because the objects are not locked
		TEST_ASSERT(db.saveObjects() == false);
		TEST_ASSERT(db.saveObject(persons[0]) == false);

		Error err;
		TEST_ASSERT(db.lockObject(persons[0], err));
		TEST_ASSERT(db.saveObject(persons[0]) == true);
		TEST_ASSERT(db.unlockObject(persons[0], err));
		TEST_ASSERT(db.saveObject(persons[0]) == false);
		TEST_ASSERT(persons[0]->lock());
		TEST_ASSERT(db.saveObject(persons[0]));
		TEST_ASSERT(persons[0]->unlock());
		TEST_ASSERT(db.saveObject(persons[0]) == false);

		TEST_ASSERT(db.lockObject(persons[1], err));
		TEST_ASSERT(db.lockObject(persons[2], err));
		TEST_ASSERT(db.lockObject(persons[3], err));

		// This change should not be saved to the database, since the object is not locked
		Person* person = dynamic_cast<Person*>(persons[0].get());
		person0Age = person->age;
		person->age = "999";
		person0ID = person->getObjectID()->get();
		TEST_ASSERT(person->isLocked() == false);
		TEST_ASSERT(persons[1]->isLocked() == true);

		// Should save only the locked objects, therefore return false because not all objects were saved
		TEST_ASSERT(db.saveObjects() == false);
	}




	TEST_FUNCTION(readBack)
	{
		TEST_START;
		JDManager db;

		TEST_ASSERT(db.setup(dbPath, dbName, dbUser));
		
		TEST_ASSERT(db.getObjectCount() == 0);
		TEST_ASSERT(db.getObjects().size() == 0);

		TEST_ASSERT(db.loadObjects());

		TEST_ASSERT(db.getObjectCount() == 4);
		TEST_ASSERT(db.getObjects().size() == 4);

		JDObject obj = db.getObject(person0ID);
		Person* person = dynamic_cast<Person*>(obj.get());
		TEST_ASSERT(person != nullptr);
		TEST_ASSERT(person->age == person0Age);
		TEST_ASSERT(person->isLocked() == false);
	}

};
