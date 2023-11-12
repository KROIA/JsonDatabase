#pragma once

#include "JD_base.h"
#include <QCoreApplication>

namespace JsonDatabase
{
	namespace Internal
	{
		namespace QTEvents
		{
			extern void processEvents();
			extern void processEvents(QEventLoop::ProcessEventsFlags flags);
			extern void processMinimalEvents();
		}
	}
}