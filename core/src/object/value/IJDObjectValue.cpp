#include "object/value/IJDObjectValue.h"
#include "object/JDObjectInterface.h"

namespace JsonDatabase
{

	void IJDObjectValue::clearChangeTransactions()
	{
		m_changeHistory.clear();
		if (m_parent)
			m_parent->onValueChanged(this);
	}

	void IJDObjectValue::onValueChange(std::shared_ptr<IChangeTransaction> change)
	{
		if (hasChanged())
		{
			m_changeHistory.push_back(change);
			if (m_parent)
				m_parent->onValueChanged(this);
		}
		else
			clearChangeTransactions();
	}
}