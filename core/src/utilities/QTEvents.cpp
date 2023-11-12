#include "utilities/QTEvents.h"


namespace JsonDatabase
{
	namespace Internal
	{
		namespace QTEvents
		{
			void processEvents()
			{
				JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
				QCoreApplication::processEvents();
			}
			void processEvents(QEventLoop::ProcessEventsFlags flags)
			{
				JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
				QCoreApplication::processEvents(flags);
			}
			void processMinimalEvents()
			{
				JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
				QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents |
												QEventLoop::ExcludeSocketNotifiers |
												QEventLoop::X11ExcludeTimers);
			}
		}
	}
}