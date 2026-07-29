#ifdef QT_ENABLED
#include <QApplication>
#endif
#include <iostream>
#include "JsonDatabase.h"
#include <iostream>
#include "tests.h"
#include <QThread>

#ifdef QT_WIDGETS_ENABLED
#include <QWidget>
#include <QApplication>
#endif

// Instantiate Tests here:
// TEST_INSTANTIATE(Test_simple); // Where Test_simple is a derived class from the Test class
TEST_INSTANTIATE(TST_stringUtilities);
//TEST_INSTANTIATE(TST_readWrite);

int main(int argc, char* argv[])
{
#ifdef QT_WIDGETS_ENABLED
	QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
	QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
#ifdef QT_ENABLED
	QApplication app(argc, argv);
#endif
	Log::UI::QConsoleView consoleView;
	consoleView.show();
	JsonDatabase::LibraryInfo::printInfo();

	QThread t;
	UnitTest::Test::TestResults results;
	QObject::connect(&t, &QThread::started, [&]()
		{
			std::cout << "Running " << UnitTest::Test::getTests().size() << " tests...\n";
			UnitTest::Test::TestResults results_local;
			UnitTest::Test::runAllTests(results_local);
			UnitTest::Test::printResults(results_local);
			results = results_local;
			
			// Send a signal to the main thread to quit the application after this thread finishes running the tests.
			// Add 100ms delay to allow the console view to update before quitting the application.
			// Use queued connection to ensure the signal is sent to the main thread.
			QMetaObject::invokeMethod(&app, [&]()
				{
					QTimer::singleShot(100, &app, &QCoreApplication::quit);
				}, Qt::QueuedConnection);
		}
	);
	t.start();
	
	app.exec();

	t.quit();
	t.wait();

	return results.getSuccess();
}
