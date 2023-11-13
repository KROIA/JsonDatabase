#pragma once

#include "JD_base.h"
#include "JDDeclaration.h"
#include <mutex>

namespace JsonDatabase
{
    namespace Internal
    {
        class JSONDATABASE_EXPORT JDManagerAysncWork
        {
            friend class JDManager;
        public:
            JDManagerAysncWork(
				JDManager& manager,
				std::mutex &mtx);
            virtual ~JDManagerAysncWork();
        

            virtual void process() = 0;
            virtual bool hasSucceeded() const = 0;
            virtual std::string getErrorMessage() const = 0;
        protected:
            JDManager& m_manager;
            std::mutex& m_mutex;

        };
    }
}
