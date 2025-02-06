#pragma once

#include "JsonDatabase_base.h"
#include "JsonDatabase_Declaration.h"
#include "Json/IJsonValue.h"
#include "changehistory/ChangeTransaction.h"
#include <vector>

namespace JsonDatabase
{
	class IJDObjectValue : public IJsonValue
	{
	public:
		IJDObjectValue(const std::string& paramName)
			: m_paramName(paramName)
		{}

		const std::string& getParamName() const
		{
			return m_paramName;
		}

		const std::vector<std::shared_ptr<IChangeTransaction>>& getChangeTransactions() const
		{
			return m_changeHistory;
		}
		void clearChangeTransactions()
		{
			m_changeHistory.clear();
		}

	protected:
		void onValueChange(std::shared_ptr<IChangeTransaction> change)
		{
			m_changeHistory.push_back(change);
		}
	private:

		const std::string m_paramName;
		std::vector<std::shared_ptr<IChangeTransaction>> m_changeHistory;

	};
}