#include <QApplication>
#include <iostream>
#include "UIWrapper.h"





int main(int argc, char* argv[])
{
	
	QApplication a(argc, argv);
	JDManager::startProfiler();
	bool ret;
	{
		UIWrapper wrapper;
		ret = a.exec();
	}
	JDManager::stopProfiler("profiler.prof");
	
	return ret;
}