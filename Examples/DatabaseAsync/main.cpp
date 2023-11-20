#include <QApplication>
#include <iostream>
#include "UIWrapper.h"





int main(int argc, char* argv[])
{
	
	QApplication a(argc, argv);
	Profiler::startProfiler();
	bool ret;
	{
		UIWrapper wrapper;
		ret = a.exec();
	}
	Profiler::stopProfiler("profiler.prof");
	
	return ret;
}