#include "manager/async/JDManagerAsyncWork.h"
#include "manager/JDManager.h"

namespace JsonDatabase
{
    namespace Internal
    {
        JDManagerAysncWork::JDManagerAysncWork(
            JDManager& manager,
            std::mutex& mtx)
            : m_manager(manager)
            , m_mutex(mtx)
        {

        }

        JDManagerAysncWork::~JDManagerAysncWork()
        {

        }
    }
}