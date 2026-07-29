#pragma once

#include "JsonDatabase_base.h"
#include "JsonDatabase_Declaration.h"
#include "Json/IJsonValue.h"
#include "changehistory/ValueChangeTransaction.h"
#include <vector>

namespace JsonDatabase
{
	class IJDObjectValue : public IJsonValue
	{
		friend class JDObjectInterface;
	public:
		IJDObjectValue(const std::string& paramName)
			: m_paramName(paramName)
		{}

		const std::string& getParamName() const
		{
			return m_paramName;
		}

		const std::vector<std::shared_ptr<IChangeTransaction>>& getValueChangeTransactions() const
		{
			return m_changeHistory;
		}
		virtual void clearValueChangeTransactions();

		/**
		 * @brief 
		 * Compares the current value with the value that was set before any change transaction occured
		 * @return true if there is a change in the value since the first change transaction
		 */
		virtual bool hasChanged() const = 0;

		/**
		 * @brief
		 * Discards all changes that were made to the value since the first change transaction
		 */
		virtual void discardChanges() = 0;

	protected:
		void onValueChange(std::shared_ptr<IChangeTransaction> change);

		void setParent(JDObjectInterface* parent)
		{
			m_parent = parent;
		}
	private:



		const std::string m_paramName;
		std::vector<std::shared_ptr<IChangeTransaction>> m_changeHistory;
		JDObjectInterface* m_parent = nullptr;

	};
}