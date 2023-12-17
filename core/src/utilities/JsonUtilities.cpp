#include "utilities/JsonUtilities.h"
#include "utilities/JDObjectIDDomain.h"
#include "object/JDObjectInterface.h"
#include "object/JDObjectRegistry.h"
#include "manager/async/WorkProgress.h"
#include "manager/JDManager.h"

#include <QJsonDocument>

#ifdef JD_ENABLE_MULTITHREADING
#include <thread>
#endif


namespace JsonDatabase
{
	namespace Utilities
	{
		
	}
}