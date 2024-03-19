#pragma once

#include "JsonDatabase_base.h"
#include "JsonDatabase_Declaration.h"
#include "manager/async/WorkProgress.h"
#include <mutex>

namespace JsonDatabase
{
    namespace Internal
    {
        class JSON_DATABASE_EXPORT JDManagerAysncWork
        {
            friend class JDManager;
            friend class JDManagerAsyncWorker;
        public:
            JDManagerAysncWork(
				JDManager& manager,
				std::mutex &mtx);
            virtual ~JDManagerAysncWork();
        

            virtual void process() = 0;
            virtual bool hasSucceeded() const = 0;
            virtual std::string getErrorMessage() const = 0;
            virtual WorkType getWorkType() const = 0;

        protected:
            JDManager& m_manager;
            std::mutex& m_mutex;
            WorkProgress m_progress;
        };
    }
}
