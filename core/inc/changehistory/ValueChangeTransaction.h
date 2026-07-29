#pragma once

#include "JsonDatabase_base.h"
#include "JsonDatabase_Declaration.h"
#include "IChangeTransaction.h"
#include <concepts>
#include <QDateTime>

namespace JsonDatabase
{
	//template <typename T>
	//concept DerivedFromIJsonValue = std::is_base_of_v<JsonValue, T>;


	//template<DerivedFromIJsonValue T>
	class ValueChangeTransaction : public IChangeTransaction
	{
		using T = JsonValue;
	public:
		ValueChangeTransaction(const std::string& identifyer, const T &oldValue, const T &newValue)
			: IChangeTransaction(identifyer)
			, m_oldValue(oldValue)
			, m_newValue(newValue)
			
		{

		}
		ValueChangeTransaction(const ValueChangeTransaction& other)
			: IChangeTransaction(other)
			, m_oldValue(other.m_oldValue)
			, m_newValue(other.m_newValue)
		{

		}
		ValueChangeTransaction(ValueChangeTransaction&& other) noexcept
			: IChangeTransaction(other)
			, m_oldValue(std::move(other.m_oldValue))
			, m_newValue(std::move(other.m_newValue))
		{

		}
		ValueChangeTransaction& operator=(const ValueChangeTransaction& other)
		{
			IChangeTransaction::operator=(other);
			m_oldValue = other.m_oldValue;
			m_newValue = other.m_newValue;
			return *this;
		}
		ValueChangeTransaction& operator=(ValueChangeTransaction&& other) noexcept
		{
			IChangeTransaction::operator=(other);
			m_oldValue = std::move(other.m_oldValue);
			m_newValue = std::move(other.m_newValue);
			return *this;
		}
		~ValueChangeTransaction()
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


		bool operator==(const ValueChangeTransaction& other) const
		{
			return m_oldValue == other.m_oldValue && m_newValue == other.m_newValue;
		}
		bool operator!=(const ValueChangeTransaction& other) const
		{
			return m_oldValue != other.m_oldValue || m_newValue != other.m_newValue;
		}

		JsonValue toJson() const override
		{
			JsonValue val = IChangeTransaction::toJson();
			JsonObject obj = val.get<JsonObject>();
			//obj["oldValue"] = m_oldValue.toJson();
			//obj["newValue"] = m_newValue.toJson();
			obj["oldValue"] = m_oldValue;
			obj["newValue"] = m_newValue;
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
					//bool success = m_oldValue.fromJson(obj.find("oldValue")->second);
					//success &= m_newValue.fromJson(obj.find("newValue")->second);
					m_oldValue = obj.find("oldValue")->second;
					m_newValue = obj.find("newValue")->second;
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