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
		ADD_TEST(TST_readWrite::loadObjectsAsync);
		ADD_TEST(TST_readWrite::saveObjectsAsync);
		ADD_TEST(TST_readWrite::multiSession);

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
	size_t objCount = 0;

	void waitUntilTimeoutOrCondition(volatile bool& condition, int timeoutMs = 10000)
	{
		while (timeoutMs > 0 && !condition)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			timeoutMs -= 100;
			// process events of this thread
			QCoreApplication::processEvents();
		}
	}

	// Tests
	TEST_FUNCTION(initialize)
	{
		TEST_START;
		JDManager db;

		TEST_ASSERT(db.setup(dbPath, dbName, dbUser));

		// Create a Person Object
		std::vector<JDObject> persons = createPersons();
		objCount = persons.size();

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

		TEST_ASSERT(db.getObjectCount() == objCount);
		TEST_ASSERT(db.getObjects().size() == objCount);

		JDObject obj = db.getObject(person0ID);
		Person* person = dynamic_cast<Person*>(obj.get());
		TEST_ASSERT(person != nullptr);
		TEST_ASSERT(person->age == person0Age);
		TEST_ASSERT(person->isLocked() == false);
	}

	TEST_FUNCTION(loadObjectsAsync)
	{
		TEST_START;
		JDManager db;

		TEST_ASSERT(db.setup(dbPath, dbName, dbUser));
		volatile bool loadSuccess = false;
		QObject::connect(&db, &JDManager::loadObjectsDone, [&loadSuccess](bool result) 
						 {
							 loadSuccess = result; 
						 });
		db.loadObjectsAsync();
		waitUntilTimeoutOrCondition(loadSuccess);

		if(!loadSuccess)
			TEST_MESSAGE("Timeout while loading objects");

		TEST_ASSERT(db.getObjectCount() == objCount);
		TEST_ASSERT(loadSuccess);
	}

	TEST_FUNCTION(saveObjectsAsync)
	{
		TEST_START;
		JDManager db;

		TEST_ASSERT(db.setup(dbPath, dbName, dbUser));
		TEST_ASSERT(db.loadObjects());
		JsonDatabase::Error lckErr;
		TEST_ASSERT(db.lockAllObjs(lckErr));
		if (lckErr != JsonDatabase::Error::none)
		{
			TEST_MESSAGE(std::string("Error while locking objects: ") + JsonDatabase::errorToString(lckErr));
		}
		volatile bool saveSuccess = false;
		QObject::connect(&db, &JDManager::saveObjectsDone, [&saveSuccess](bool result)
						 {
							 saveSuccess = result;
						 });
		db.saveObjectsAsync();
		waitUntilTimeoutOrCondition(saveSuccess);

		if (!saveSuccess)
			TEST_MESSAGE("Timeout while loading objects");

		TEST_ASSERT(db.getObjectCount() == objCount);
		TEST_ASSERT(saveSuccess);
	}

	TEST_FUNCTION(multiSession)
	{
		TEST_START;
		JDManager db1;
		JDManager db2;

		TEST_ASSERT(db1.setup(dbPath, dbName, dbUser));
		TEST_ASSERT(db2.setup(dbPath, dbName, dbUser));

		TEST_ASSERT(db1.loadObjects());
		TEST_ASSERT(db2.loadObjects());

		JsonDatabase::Error lckErr;
		TEST_ASSERT(db1.lockAllObjs(lckErr));
		if (lckErr != JsonDatabase::Error::none)
		{
			TEST_MESSAGE(std::string("Error while locking objects: ") + JsonDatabase::errorToString(lckErr));
		}
		TEST_ASSERT(db1.saveObjects());
		TEST_ASSERT(db2.loadObjects());
		TEST_ASSERT(db2.getObjectCount() == objCount);
		TEST_ASSERT(db2.getObjects().size() == objCount);
		TEST_ASSERT(db2.getObject(person0ID) != nullptr);
		Person* person = dynamic_cast<Person*>(db2.getObject(person0ID).get());
		TEST_ASSERT(person != nullptr);
		TEST_ASSERT(person->age == person0Age);
		TEST_ASSERT(person->isLocked() == true);
	}

};
