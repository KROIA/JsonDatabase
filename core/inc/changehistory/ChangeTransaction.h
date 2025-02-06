#pragma once

#include "JsonDatabase_base.h"
#include "JsonDatabase_Declaration.h"
#include "IChangeTransaction.h"
#include <concepts>
#include <QDateTime>

namespace JsonDatabase
{
	template <typename T>
	concept DerivedFromIJsonValue = std::is_base_of_v<IJsonValue, T>;


	template<DerivedFromIJsonValue T>
	class ChangeTransaction : public IChangeTransaction
	{
	public:
		ChangeTransaction(const std::string& identifyer, const T &oldValue, const T &newValue)
			: IChangeTransaction(identifyer)
			, m_oldValue(oldValue)
			, m_newValue(newValue)
			
		{

		}
		ChangeTransaction(const ChangeTransaction& other)
			: IChangeTransaction(other)
			, m_oldValue(other.m_oldValue)
			, m_newValue(other.m_newValue)
		{

		}
		ChangeTransaction(ChangeTransaction&& other) noexcept
			: IChangeTransaction(other)
			, m_oldValue(std::move(other.m_oldValue))
			, m_newValue(std::move(other.m_newValue))
		{

		}
		ChangeTransaction& operator=(const ChangeTransaction& other)
		{
			IChangeTransaction::operator=(other);
			m_oldValue = other.m_oldValue;
			m_newValue = other.m_newValue;
			return *this;
		}
		ChangeTransaction& operator=(ChangeTransaction&& other) noexcept
		{
			IChangeTransaction::operator=(other);
			m_oldValue = std::move(other.m_oldValue);
			m_newValue = std::move(other.m_newValue);
			return *this;
		}
		~ChangeTransaction()
		{

		}

		const T& getOldValue() const
		{
			return m_oldValue;
		}
		const T& getNewValue() const
		{
			return m_newValue;
		}


		bool operator==(const ChangeTransaction& other) const
		{
			return m_oldValue == other.m_oldValue && m_newValue == other.m_newValue;
		}
		bool operator!=(const ChangeTransaction& other) const
		{
			return m_oldValue != other.m_oldValue || m_newValue != other.m_newValue;
		}

		JsonValue toJson() const override
		{
			JsonValue val = IChangeTransaction::toJson();
			JsonObject obj = val.get<JsonObject>();
			obj["oldValue"] = m_oldValue.toJson();
			obj["newValue"] = m_newValue.toJson();
			return obj;
		}
		bool fromJson(const JsonValue& value) override
		{
			bool success = IChangeTransaction::fromJson(value);
			if (value.holds<JsonObject>() && success)
			{
				const JsonObject& obj = value.get<JsonObject>();
				if (obj.contains("oldValue") && 
					obj.contains("newValue"))
				{
					bool success = m_oldValue.fromJson(obj.find("oldValue")->second);
					success &= m_newValue.fromJson(obj.find("newValue")->second);
					return success;
				}
			}
			return false;
		}

	private:
		T m_oldValue;
		T m_newValue;
		
	};
}