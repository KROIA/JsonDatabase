#pragma once

#include "JsonDatabase_base.h"
#include "JsonDatabase_Declaration.h"
#include "JDObjectValue.h"

namespace JsonDatabase
{
	template<class T>
	class JDObjectValue : public IJDObjectValue
	{
		/*class JDObjectValueContainer : public IJsonValue
		{
		public:
			JDObjectValueContainer(const T &var)
				: var(var)
			{}
			JsonValue toJson() const
			{
				return JsonValue(var);
			}
			bool fromJson(const JsonValue& value)
			{
				if (!value.holds<T>())
					return false;
				
				var = value.get<T>();
				return true;
			}
		private:
			T var;
		};*/
	public:
		

		JDObjectValue(const std::string& paramName)
			: IJDObjectValue(paramName)
		{
			m_value = T();
			m_orgValue = m_value;
		}
		JDObjectValue(const std::string& paramName, const T& value)
			: IJDObjectValue(paramName)
		{
			m_value = value;
			m_orgValue = m_value;
		}
		JDObjectValue(const std::string& paramName, T&& value) noexcept
			: IJDObjectValue(paramName)
		{
			m_value = std::move(value);
			m_orgValue = m_value;
		}
		JDObjectValue(const JDObjectValue& other)
			: IJDObjectValue(other)
		{
			m_value = other.m_value;
			m_orgValue = other.m_orgValue;
		}
		JDObjectValue(JDObjectValue&& other) noexcept
			: IJDObjectValue(other)
		{
			m_value = std::move(other.m_value);
			m_orgValue = std::move(other.m_orgValue);
		}
		~JDObjectValue()
		{

		}

		JDObjectValue& operator=(const JDObjectValue& other)
		{
			if (m_value == other.m_value)
				return *this;
			T old = m_value;
			m_value = other.m_value;
			onValueChange(std::make_shared<ValueChangeTransaction>(getParamName(), JsonValue(old), JsonValue(m_value)));
			return *this;
		}
		JDObjectValue& operator=(const T& other)
		{
			if (m_value == other)
				return *this;
			T old = m_value;
			m_value = other;
			onValueChange(std::make_shared<ValueChangeTransaction>(getParamName(), JsonValue(old), JsonValue(m_value)));
			return *this;
		}

		operator T() const
		{
			return m_value;
		}
		
		T operator+(const JDObjectValue& other) const
		{
			return m_value + other.m_value;
		}
		T operator-(const JDObjectValue& other) const
		{
			return m_value - other.m_value;
		}
		T operator*(const JDObjectValue& other) const
		{
			return m_value * other.m_value;
		}
		T operator/(const JDObjectValue& other) const
		{
			return m_value / other.m_value;
		}

		T operator+(const T& other) const
		{
			return m_value + other;
		}
		T operator-(const T& other) const
		{
			return m_value - other;
		}
		T operator*(const T& other) const
		{
			return m_value * other;
		}
		T operator/(const T& other) const
		{
			return m_value / other;
		}

		// Enable this function only if T is std::string
		template <typename U = T, typename std::enable_if<std::is_same<U, std::string>::value, int>::type = 0>
		U operator+(const char* str) const {
			return m_value + (str);
		}



		bool operator==(const JDObjectValue& other) const
		{
			return m_value == other.m_value;
		}
		bool operator!=(const JDObjectValue& other) const
		{
			return m_value != other.m_value;
		}

		bool operator==(const T& other) const
		{
			return m_value == other;
		}
		bool operator!=(const T& other) const
		{
			return m_value != other;
		}

		bool operator<(const JDObjectValue& other) const
		{
			return m_value < other.m_value;
		}
		bool operator>(const JDObjectValue& other) const
		{
			return m_value > other.m_value;
		}
		bool operator<=(const JDObjectValue& other) const
		{
			return m_value <= other.m_value;
		}
		bool operator>=(const JDObjectValue& other) const
		{
			return m_value >= other.m_value;
		}

		bool operator<(const T& other) const
		{
			return m_value < other;
		}
		bool operator>(const T& other) const
		{
			return m_value > other;
		}
		bool operator<=(const T& other) const
		{
			return m_value <= other;
		}
		bool operator>=(const T& other) const
		{
			return m_value >= other;
		}

		T getValue() const
		{
			return m_value;
		}



		JsonValue toJson() const override
		{
			return JsonValue(m_value);
		}
		bool fromJson(const JsonValue& value) override
		{
			if (value.holds<T>())
			{
				m_value = value.get<T>();
				clearValueChangeTransactions();
				return true;
			}
			return false;
		}

		void clearValueChangeTransactions() override
		{
			IJDObjectValue::clearValueChangeTransactions();
			m_orgValue = m_value;
		}

		bool hasChanged() const override
		{
			return m_value != m_orgValue;
		}

		void discardChanges() override
		{
			m_value = m_orgValue;
			clearValueChangeTransactions();
		}

	private:
		T m_value;
		T m_orgValue; // The original value before any change transaction occured
	};

	// Non-member operator+ to allow std::string + JDObjectValue<std::string>
	template <typename T, typename std::enable_if<std::is_same<T, std::string>::value, int>::type = 0>
	std::string operator+(const std::string& lhs, const JDObjectValue<T>& rhs) {
		return lhs + rhs.getValue();
	}

}