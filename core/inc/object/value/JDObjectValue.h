#pragma once

#include "JsonDatabase_base.h"
#include "JsonDatabase_Declaration.h"
#include "JDObjectValue.h"

namespace JsonDatabase
{
	template<class T>
	class JDObjectValue : public IJDObjectValue
	{
		class JDObjectValueContainer : public IJsonValue
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
		};
	public:
		

		JDObjectValue(const std::string& paramName)
			: IJDObjectValue(paramName)
		{
			m_value = T();
		}
		JDObjectValue(const std::string& paramName, const T& value)
			: IJDObjectValue(paramName)
		{
			m_value = value;
		}
		JDObjectValue(const std::string& paramName, T&& value) noexcept
			: IJDObjectValue(paramName)
		{
			m_value = std::move(value);
		}
		JDObjectValue(const JDObjectValue& other)
			: IJDObjectValue(other)
		{
			m_value = other.m_value;
		}
		JDObjectValue(JDObjectValue&& other) noexcept
			: IJDObjectValue(other)
		{
			m_value = std::move(other.m_value);
		}
		~JDObjectValue()
		{

		}

		JDObjectValue& operator=(const JDObjectValue& other)
		{
			if (m_value == other.m_value)
				return *this;
			onValueChange(std::make_shared<ChangeTransaction<JDObjectValueContainer>>(getParamName(), m_value, other.m_value));
			m_value = other.m_value;
			return *this;
		}
		JDObjectValue& operator=(const T& other)
		{
			if (m_value == other)
				return *this;
			onValueChange(std::make_shared<ChangeTransaction<JDObjectValueContainer>>(getParamName(), m_value, other));
			m_value = other;
			return *this;
		}

		/*
		JDObjectValue& operator=(JDObjectValue&& other) noexcept
		{
			onValueChange(std::make_shared<ChangeTransaction<T>>(getParamName(), m_value, other.m_value));
			m_value = std::move(other.m_value);
			return *this;
		}
		
		JDObjectValue& operator=(T&& other) noexcept
		{
			onValueChange(std::make_shared<ChangeTransaction<T>>(getParamName(), m_value, other));
			m_value = std::move(other);
			return *this;
		}*/

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
				return true;
			}
			return false;
		}

	private:
		T m_value;
	};

	// Non-member operator+ to allow std::string + Holder<std::string>
	template <typename T, typename std::enable_if<std::is_same<T, std::string>::value, int>::type = 0>
	std::string operator+(const std::string& lhs, const JDObjectValue<T>& rhs) {
		return lhs + rhs.getValue();
	}

}
/*
// Non-member operator+ to allow std::string + Holder<std::string>
template <typename T, typename std::enable_if<std::is_same<T, std::string>::value, int>::type = 0>
std::string operator+(const std::string& lhs, const JDObjectValue<T>& rhs) {
	return lhs + rhs.get();
}*/

