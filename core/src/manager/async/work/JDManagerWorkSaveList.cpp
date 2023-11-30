#include "manager/async/work/JDManagerWorkSaveList.h"
#include "manager/JDManager.h"
#include "utilities/JDUniqueMutexLock.h"
#include "utilities/AsyncContextDrivenDeleter.h"

namespace JsonDatabase
{
	namespace Internal
	{
		JDManagerAysncWorkSaveList::JDManagerAysncWorkSaveList(
			JDManager& manager,
			std::mutex& mtx,
			const std::vector<JDObject>& objects)
			: JDManagerAysncWork(manager, mtx)
			, m_success(false)
		{
			m_objects.resize(objects.size());
			for (size_t i = 0; i < objects.size(); ++i)
			{
				//objects[i]->incrementVersionValue();
				m_objects[i] = manager.createClone(objects[i]);
				//m_objects[i] = objects[i]->clone();
			}
				
			m_progress.setTaskName("Speichere "+ std::to_string(m_objects.size())+ " Objekte");
		}
		JDManagerAysncWorkSaveList::~JDManagerAysncWorkSaveList()
		{
			// Delete all objects asynchroneously
			AsyncContextDrivenDeleter asyncDeleter(m_objects);
			m_objects.clear();
		}
		bool JDManagerAysncWorkSaveList::hasSucceeded() const
		{
			return m_success; 
		}
		void JDManagerAysncWorkSaveList::process()
		{
			JD_ASYNC_WORKER_PROFILING_FUNCTION(JD_COLOR_STAGE_4);
			//JDM_UNIQUE_LOCK_P;
			m_success = m_manager.saveObjects_internal(m_objects, JDManager::s_fileLockTimeoutMs, &m_progress);
		}
		std::string JDManagerAysncWorkSaveList::getErrorMessage() const
		{
			if (m_success)
				return "";
			return "Failed to save all objects";
		}
		WorkType JDManagerAysncWorkSaveList::getWorkType() const
		{
			return WorkType::saveAllObjects;
		}
	}
}