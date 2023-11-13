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
			JDM_UNIQUE_LOCK_P;
			m_success = m_manager.loadObject_internal(m_object);
		}
		std::string JDManagerAysncWorkLoadSingleObject::getErrorMessage() const
		{
			if (m_success)
				return "";
			return "Failed to load the object";
		}
	}
}