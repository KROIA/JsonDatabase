#pragma once

#include "JsonDatabase_base.h"
#include "JsonDatabase_Declaration.h"
#include "IChangeTransaction.h"
#include <concepts>
#include <QDateTime>

namespace JsonDatabase
{
	class ChangeTransaction : public IChangeTransaction
	{
	public:
		ChangeTransaction(const std::string& identifyer, const std::string &msg)
			: IChangeTransaction(identifyer)
			, m_message(msg)

		{

		}
		ChangeTransaction(const ChangeTransaction& other)
			: IChangeTransaction(other)
			, m_message(other.m_message)
		{

		}
		ChangeTransaction(ChangeTransaction&& other) noexcept
			: IChangeTransaction(other)
			, m_message(std::move(other.m_message))
		{

		}
		ChangeTransaction& operator=(const ChangeTransaction& other)
		{
			IChangeTransaction::operator=(other);
			m_message = other.m_message;
			return *this;
		}
		ChangeTransaction& operator=(ChangeTransaction&& other) noexcept
		{
			IChangeTransaction::operator=(other);
			m_message = std::move(other.m_message);
			return *this;
		}
		~ChangeTransaction()
		{

		}

		const std::string& getMessage() const
		{
			return m_message;
		}


		bool operator==(const ChangeTransaction& other) const
		{
			return m_message == other.m_message;
		}
		bool operator!=(const ChangeTransaction& other) const
		{
			return m_message != other.m_message;
		}

		JsonValue toJson() const override
		{
			JsonValue val = IChangeTransaction::toJson();
			JsonObject obj = val.get<JsonObject>();
			obj["msg"] = m_message;
			return obj;
		}
		bool fromJson(const JsonValue& value) override
		{
			bool success = IChangeTransaction::fromJson(value);
			if (value.holds<JsonObject>() && success)
			{
				const JsonObject& obj = value.get<JsonObject>();
				if (obj.contains("msg"))
				{
					m_message = obj.find("msg")->second.get<std::string>();
					return success;
				}
			}
			return false;
		}

	private:
		std::string m_message;
	};
}