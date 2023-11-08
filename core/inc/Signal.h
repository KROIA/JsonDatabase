#pragma once

#include "JD_base.h"
#include <functional>
#include <vector>

namespace JsonDatabase
{
        // Define a Signal class
    template<typename... Args>
    class Signal
    {
    public:
        using SlotFunction = std::function<void(Args...)>;

        void connectSlot(const SlotFunction& slot) 
        {
            m_slotsWithArgs.emplace_back(slot);
        }
        void disconnectSlot(const SlotFunction &slot) 
        {
            for (size_t i = 0; i < m_slotsWithArgs.size(); i++)
            {
                if (getAddress(m_slotsWithArgs[i]) == getAddress(slot))
                {
                    m_slotsWithArgs.erase(m_slotsWithArgs.begin() + i);
                    break;
                }
            }
        }
        void disconnectAll()
        {
            m_slotsWithArgs.clear();
        }

        size_t getSlotCount() const 
		{
			return m_slotsWithArgs.size();
		}

        void emitSignal(Args... args) 
        {
            for (const auto& slot : m_slotsWithArgs) {
                slot(args...);
            }
        }

    private:
        size_t getAddress(const SlotFunction& f) 
        {
            typedef void(fnType)(Args...);
            const fnType* fnPointer = *f.target< fnType>();
            return reinterpret_cast<size_t>(fnPointer);
        }


        std::vector<SlotFunction> m_slotsWithArgs;
    };
}