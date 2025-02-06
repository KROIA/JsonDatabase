#pragma once

#include "JsonDatabase_base.h"
#include "JsonDatabase_Declaration.h"
#include "Json/IJsonValue.h"
#include <QDateTime>
#include <variant>

namespace JsonDatabase
{
	class IChangeTransaction : public IJsonValue
	{
	public:
		IChangeTransaction(const std::string &identifyer)
			: m_identifyer(identifyer)
			, m_timestamp(QDateTime::currentDateTime())
		{

		}
		IChangeTransaction(const IChangeTransaction& other)
			: m_identifyer(other.m_identifyer)
			, m_timestamp(other.m_timestamp)
		{

		}
		IChangeTransaction(IChangeTransaction&& other) noexcept
			: m_identifyer(other.m_identifyer)
			, m_timestamp(other.m_timestamp)
		{

		}
		IChangeTransaction& operator=(const IChangeTransaction& other)
		{
			m_identifyer = other.m_identifyer;
			m_timestamp = other.m_timestamp;
			return *this;
		}
		IChangeTransaction& operator=(IChangeTransaction&& other) noexcept
		{
			m_identifyer = std::move(other.m_identifyer);
			m_timestamp = std::move(other.m_timestamp);
			return *this;
		}


		bool isNewerThan(const QDateTime& timestamp) const
		{
			return m_timestamp > timestamp;
		}
		bool isOlderThan(const QDateTime& timestamp) const
		{
			return m_timestamp < timestamp;
		}
		const QDateTime& getTimestamp() const
		{
			return m_timestamp;
		}
		const std::string& getIdentifyer() const
		{
			return m_identifyer;
		}


		JsonValue toJson() const override
		{
			JsonObject obj;
			obj["identifyer"] = m_identifyer;
			obj["timestamp"] = m_timestamp.toString(Qt::ISODate).toStdString();
			return obj;
		}
		bool fromJson(const JsonValue& value) override
		{
			if (value.holds<JsonObject>())
			{
				const JsonObject& obj = value.get<JsonObject>();
				if(!obj.contains("timestmp") ||
					!obj.contains("identifyer"))
					return false;
				auto timestmpIt = obj.find("timestmp");
				auto identifyerIt = obj.find("identifyer");
				if (!timestmpIt->second.holds<std::string>() ||
					!identifyerIt->second.holds<std::string>())
					return false;

				std::string timestamp = timestmpIt->second.get<std::string>();
				m_timestamp = QDateTime::fromString(timestamp.c_str(), Qt::ISODate);
				m_identifyer = identifyerIt->second.get<std::string>();
			}
			return false;
		}


		template<typename T>
		static std::vector<T> combine(const std::vector<std::vector<T>>& list)
		{
			std::vector<T> result;
			for (const std::vector<T>& vec : list)
			{
				result.insert(result.end(), vec.begin(), vec.end());
			}
			return result;
		}
		template<typename T>
		static std::vector<T> combine(const std::vector<T>& list1, const std::vector<T>& list2)
		{
			std::vector<T> result = list1;
			result.insert(result.end(), list2.begin(), list2.end());
			return result;
		}

		static std::vector<std::shared_ptr<IChangeTransaction>> sortByDate(const std::vector<std::shared_ptr<IChangeTransaction>>& list)
		{
			std::vector<std::shared_ptr<IChangeTransaction>> result = list;
			std::sort(result.begin(), result.end(),
				[](const std::shared_ptr<IChangeTransaction>& a, const std::shared_ptr<IChangeTransaction>& b)
				{ return a->getTimestamp() < b->getTimestamp(); });
			return result;
		}

		static JsonValue toJson(const std::vector<std::shared_ptr<IChangeTransaction>>& list)
		{
			JsonArray arr;
			for (const std::shared_ptr< IChangeTransaction>& t : list)
			{
				arr.push_back(t->toJson());
			}
			return JsonValue(arr);
		}

	protected:
		std::string m_identifyer;
		QDateTime m_timestamp;
	};
}