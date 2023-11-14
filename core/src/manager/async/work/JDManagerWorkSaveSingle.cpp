#include "manager/async/work/JDManagerWorkSaveSingle.h"
#include "manager/JDManager.h"
#include "utilities/JDUniqueMutexLock.h"

namespace JsonDatabase
{
	namespace Internal
	{
		JDManagerAysncWorkSaveSingle::JDManagerAysncWorkSaveSingle(
			JDManager& manager,
			std::mutex& mtx,
			JDObjectInterface* object)
			: JDManagerAysncWork(manager, mtx)
			, m_object(object->clone())
			, m_success(false)
		{

		}
		JDManagerAysncWorkSaveSingle::~JDManagerAysncWorkSaveSingle()
		{
			delete m_object;
		}
		bool JDManagerAysncWorkSaveSingle::hasSucceeded() const
		{
			return m_success; 
		}
		void JDManagerAysncWorkSaveSingle::process()
		{
			JDM_UNIQUE_LOCK_P;
			m_success = m_manager.saveObject_internal(m_object, JDManager::s_fileLockTimeoutMs);
		}
		std::string JDManagerAysncWorkSaveSingle::getErrorMessage() const
		{
			if (m_success)
				return "";
			return "Failed to save the object";
		}
	}
}