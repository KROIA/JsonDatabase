#include "manager/async/work/JDManagerWorkLoadSingleObject.h"
#include "manager/JDManager.h"
#include "utilities/JDUniqueMutexLock.h"

namespace JsonDatabase
{
	namespace Internal
	{
		JDManagerAysncWorkLoadSingleObject::JDManagerAysncWorkLoadSingleObject(
			JDManager& manager,
			std::mutex& mtx,
			JDObjectInterface* object)
			: JDManagerAysncWork(manager, mtx)
			, m_object(object)
			, m_success(false)
		{
			if (object)
			{
				m_progress.setTaskName("Lade Objekt: " + m_object->getObjectID()->toString());
			}
		}
		JDManagerAysncWorkLoadSingleObject::~JDManagerAysncWorkLoadSingleObject()
		{

		}
		bool JDManagerAysncWorkLoadSingleObject::hasSucceeded() const
		{ 
			return m_success; 
		}
		JDObjectInterface* JDManagerAysncWorkLoadSingleObject::getObject() const
		{
			return m_object;
		}
		void JDManagerAysncWorkLoadSingleObject::process()
		{
			JD_ASYNC_WORKER_PROFILING_FUNCTION(JD_COLOR_STAGE_4);
			{
				JDM_UNIQUE_LOCK_P;
				if (!m_object)
				{
					m_success = false;
					return;
				}
				m_success = m_manager.loadObject_internal(m_object, &m_progress);
			}
		}
		std::string JDManagerAysncWorkLoadSingleObject::getErrorMessage() const
		{
			if (m_success)
				return "";
			return "Failed to load the object";
		}
		WorkType JDManagerAysncWorkLoadSingleObject::getWorkType() const
		{
			return WorkType::loadSingleObject;
		}
	}
}