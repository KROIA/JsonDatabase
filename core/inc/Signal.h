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

        Signal(const std::string &name)
            : m_name(name)
//#ifdef JD_PROFILING
            , m_profilerName("Emit signal: " + m_name)
//#endif
        {}

        const std::string& getName() const
        {
			return m_name;
        }

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

        void emitSignal(Args... args) const
        {
            JD_SIGNAL_PROFILING_BLOCK(m_profilerName.c_str(), JD_COLOR_STAGE_10);
            for (const auto& slot : m_slotsWithArgs) {
                JD_SIGNAL_PROFILING_BLOCK("Slot", JD_COLOR_STAGE_11);
                slot(args...);
            }
        }

    private:
        size_t getAddress(const SlotFunction& f) const
        {
            typedef void(fnType)(Args...);
            const fnType* fnPointer = *f.target< fnType>();
            return reinterpret_cast<size_t>(fnPointer);
        }

        const std::string m_name;
//#ifdef JD_PROFILING
        const std::string m_profilerName;
//#endif
        std::vector<SlotFunction> m_slotsWithArgs;
    };
}