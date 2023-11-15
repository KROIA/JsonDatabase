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
			, m_object(nullptr)
			, m_success(false)
		{
			if (object)
			{
				m_object = object->clone();
				m_progress.setTaskName("Speichere Objekt: " + m_object->getObjectID());
			}
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
			if (!m_object)
			{
				m_success = false;
				return;
			}
			m_success = m_manager.saveObject_internal(m_object, JDManager::s_fileLockTimeoutMs, &m_progress);
		}
		std::string JDManagerAysncWorkSaveSingle::getErrorMessage() const
		{
			if (m_success)
				return "";
			return "Failed to save the object";
		}
		WorkType JDManagerAysncWorkSaveSingle::getWorkType() const
		{
			return WorkType::saveSingleObject;
		}
	}
}