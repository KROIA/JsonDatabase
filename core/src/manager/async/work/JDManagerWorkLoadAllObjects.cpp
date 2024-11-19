#include "manager/async/work/JDManagerWorkLoadAllObjects.h"
#include "manager/JDManager.h"
#include "utilities/JDUniqueMutexLock.h"

namespace JsonDatabase
{
	namespace Internal
	{
		JDManagerAysncWorkLoadAllObjects::JDManagerAysncWorkLoadAllObjects(
			JDManager& manager,
			std::mutex& mtx,
			int mode)
			: JDManagerAysncWork(manager, mtx)
			, m_success(false)
			, m_loadMode(mode)
		{
			m_progress.setTaskName("Lade alle Objekte");
		}
		JDManagerAysncWorkLoadAllObjects::~JDManagerAysncWorkLoadAllObjects()
		{

		}
		bool JDManagerAysncWorkLoadAllObjects::hasSucceeded() const
		{
			return m_success; 
		}
		void JDManagerAysncWorkLoadAllObjects::process()
		{
			JD_ASYNC_WORKER_PROFILING_FUNCTION(JD_COLOR_STAGE_4);
			m_success = m_manager.loadObjects(m_loadMode, &m_progress);
		}
		std::string JDManagerAysncWorkLoadAllObjects::getErrorMessage() const
		{
			if(m_success)
				return "";
			return "Failed to load all objects";
		}
		WorkType JDManagerAysncWorkLoadAllObjects::getWorkType() const
		{
			return WorkType::loadAllObjects;
		}
	}
}
