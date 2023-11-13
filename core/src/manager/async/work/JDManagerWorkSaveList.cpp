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
			, m_objects(objects)
			, m_success(false)
		{

		}
		JDManagerAysncWorkSaveList::~JDManagerAysncWorkSaveList()
		{

		}
		bool JDManagerAysncWorkSaveList::hasSucceeded() const
		{
			return m_success; 
		}
		void JDManagerAysncWorkSaveList::process()
		{
			JDM_UNIQUE_LOCK_P;
			m_success = m_manager.saveObjects_internal(m_objects, JDManager::s_fileLockTimeoutMs);
		}
		std::string JDManagerAysncWorkSaveList::getErrorMessage() const
		{
			if (m_success)
				return "";
			return "Failed to save all objects";
		}
	}
}