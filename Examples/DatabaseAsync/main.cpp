
#include <QApplication>
#include <iostream>
#include "UIWrapper.h"
#include "JsonDatabase.h"



int main(int argc, char* argv[])
{
	QApplication a(argc, argv);
	Profiler::start();

	bool ret;
	{
		UIWrapper wrapper;
		ret = a.exec();
	}
	Profiler::stop("profiler.prof");
	return ret;
}
