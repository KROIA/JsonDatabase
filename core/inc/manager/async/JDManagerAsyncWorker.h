#pragma once

#include "JsonDatabase_base.h"
#include "JsonDatabase_Declaration.h"
#include "manager/async/WorkProgress.h"
#include <vector>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>

namespace JsonDatabase
{
    namespace Internal
    {
        class JSON_DATABASE_EXPORT_EXPORT JDManagerAsyncWorker
        {
            friend JDManager;
            JDManagerAsyncWorker(
                JDManager& manager,
                std::mutex& mtx);
            ~JDManagerAsyncWorker();
            void setup();
            void addWork(std::shared_ptr<JDManagerAysncWork> work);
            bool isWorkDone(std::shared_ptr<JDManagerAysncWork> work);
            void removeDoneWork(std::shared_ptr<JDManagerAysncWork> work);
            void clearDoneWork();
            void process();
            void start();
            void stop();
        public:

            WorkProgress getWorkProgress() const;
            bool isBusy() const;
            WorkType getCurrentWorkType() const;

        private:
            void threadLoop();
            void processWork(const std::vector<std::shared_ptr<JDManagerAysncWork>>& workList);
            void processWork(std::shared_ptr<JDManagerAysncWork> work);

            JDManager& m_manager;
            std::mutex& m_mutex;

            std::thread* m_thread;
            std::vector<std::shared_ptr<JDManagerAysncWork>> m_workList;
            std::vector<std::shared_ptr<JDManagerAysncWork>> m_workListDone;
            std::mutex m_mutexInternal;
            std::mutex m_workListMutex;
            std::condition_variable m_cv;
            std::atomic<bool> m_stopFlag;
            std::atomic<bool> m_busy;

            std::atomic<std::shared_ptr<JDManagerAysncWork>> m_currentWork;
            
        };
    }
}
