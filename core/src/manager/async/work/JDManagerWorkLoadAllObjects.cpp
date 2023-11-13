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
			JDM_UNIQUE_LOCK_P;
			m_success = m_manager.loadObjects_internal(m_loadMode);
		}
		std::string JDManagerAysncWorkLoadAllObjects::getErrorMessage() const
		{
			if(m_success)
				return "";
			return "Failed to load all objects";
		}
	}
}
