#include "object/JDObjectManager.h"
#include "object/JDObjectInterface.h"

namespace JsonDatabase
{
	namespace Internal
	{
		JDObjectManager::JDObjectManager(const JDObject& obj, const JDObjectIDptr& id)
			: m_obj(obj)
			, m_id(id)
			, m_lockstate(Lockstate::unlocked)
			, m_changestate(ChangeState::unchanged)
		{
			m_obj->setManager(this);
		}
		JDObjectManager::~JDObjectManager()
		{
			m_obj->setManager(nullptr);
		}


		const JDObjectIDptr& JDObjectManager::getID() const
		{
			return m_id;
		}
		const JDObject& JDObjectManager::getObject() const
		{
			return m_obj;
		}
		Lockstate JDObjectManager::getLockstate() const
		{
			return m_lockstate;
		}
		ChangeState JDObjectManager::getChangeState() const
		{
			return m_changestate;
		}
	}
}