#include <QCoreApplication>
#include <QTimer>

// Include for the Database Data Objects
#include "Person.h"

// JD Database
#include "JsonDatabase.h"

// Include Logger for debugging
#include "Logger.h"



int main(int argc, char* argv[])
{
	QCoreApplication a(argc, argv);
	Log::UI::NativeConsoleView consoleView;
	Log::LogObject logger("main");

	// Create a Database
	JsonDatabase::JDManager db("SandboxDB","DBName","User");

	// Setup the Database
	bool setup = db.setup();
	if (!setup)
	{
		logger.logError("Failed to setup the Database");
	}
	else
	{
		logger.logInfo("Database setup successful");

		// Create a Person Object
		std::vector<JDObject> persons = createPersons();

		// Save the Person Object to the Database
		db.addObject(persons);
		db.saveObjects();
		
		
		db.loadObjects();
		db.saveObjects();

	}

	QTimer::singleShot(1000, &a, [&]{db.stop(); });


	int ret = a.exec();
	db.stop();
	getchar();
	return ret;
}