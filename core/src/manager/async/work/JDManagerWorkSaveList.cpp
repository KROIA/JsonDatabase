#include "manager/async/work/JDManagerWorkSaveList.h"
#include "manager/JDManager.h"
#include "utilities/JDUniqueMutexLock.h"

namespace JsonDatabase
{
	namespace Internal
	{
		JDManagerAysncWorkSaveList::JDManagerAysncWorkSaveList(
			JDManager& manager,
			std::mutex& mtx,
			const std::vector<JDObjectInterface*>& objects)
			: JDManagerAysncWork(manager, mtx)
			, m_success(false)
		{
			m_objects.resize(objects.size());
			for (size_t i = 0; i < objects.size(); ++i)
				m_objects[i] = objects[i]->clone();
				
			m_progress.setTaskName("Speichere "+ std::to_string(m_objects.size())+ " Objekte");
		}
		JDManagerAysncWorkSaveList::~JDManagerAysncWorkSaveList()
		{
			for (size_t i = 0; i < m_objects.size(); ++i)
				delete m_objects[i];
		}
		bool JDManagerAysncWorkSaveList::hasSucceeded() const
		{
			return m_success; 
		}
		void JDManagerAysncWorkSaveList::process()
		{
			JDM_UNIQUE_LOCK_P;
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