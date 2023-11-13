#include <QApplication>
#include <iostream>
#include "MainWindow.h"



int main(int argc, char* argv[])
{
	
	QApplication a(argc, argv);
	JDManager::startProfiler();
	bool ret;
	{
		MainWindow w1("User1");
		MainWindow w2("User2");

		w1.show();
		w2.show();
		ret = a.exec();
	}
	JDManager::stopProfiler("profiler.prof");
	
	return ret;
}