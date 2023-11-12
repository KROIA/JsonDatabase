#pragma once

#include "JD_base.h"
#include "JDDeclaration.h"
#include <vector>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace JsonDatabase
{
    namespace Internal
    {
        class JSONDATABASE_EXPORT JDManagerAsyncWorker
        {
            friend JDManager;
            JDManagerAsyncWorker(
                JDManager& manager,
                std::mutex& mtx);
            ~JDManagerAsyncWorker();
            void setup();
            void addWork(JDManagerAysncWork* work);
            bool isWorkDone(JDManagerAysncWork* work);
            void removeDoneWork(JDManagerAysncWork* work);
            void clearDoneWork();
            void process();
            void start();
            void stop();
        public:


            

        private:
            void threadLoop();
            void processWork(const std::vector<JDManagerAysncWork*>& workList);
            void processWork(JDManagerAysncWork* work);

            JDManager& m_manager;
            std::mutex& m_mutex;

            std::thread* m_thread;
            std::vector<JDManagerAysncWork*> m_workList;
            std::vector<JDManagerAysncWork*> m_workListDone;
            std::mutex m_mutexInternal;
            std::condition_variable m_cv;
            std::atomic<bool> m_stopFlag;

            
        };
    }
}
