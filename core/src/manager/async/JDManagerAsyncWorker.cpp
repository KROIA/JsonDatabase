#include "manager/async/JDManagerAsyncWorker.h"
#include "manager/JDManager.h"

namespace JsonDatabase
{
    namespace Internal
    {
        JDManagerAsyncWorker::JDManagerAsyncWorker(
            JDManager& manager,
            std::mutex& mtx)
            : m_manager(manager)
            , m_mutex(mtx)
            , m_thread(nullptr)
        {

        }
        JDManagerAsyncWorker::~JDManagerAsyncWorker()
        {
            stop();
        }
        void JDManagerAsyncWorker::setup()
        {
            start();
        }

        void JDManagerAsyncWorker::addWork(JDManagerAysncWork* work)
        {
            JDM_UNIQUE_LOCK_M(m_mutexInternal);
            m_workList.push_back(work);
        }
        bool JDManagerAsyncWorker::isWorkDone(JDManagerAysncWork* work)
        {
            JDM_UNIQUE_LOCK_M(m_mutexInternal);
            return std::find(m_workListDone.begin(), m_workListDone.end(), work) != m_workListDone.end();
        }
        void JDManagerAsyncWorker::removeDoneWork(JDManagerAysncWork* work)
        {
            JDM_UNIQUE_LOCK_M(m_mutexInternal);
            auto it = std::find(m_workListDone.begin(), m_workListDone.end(), work);
            if (it != m_workListDone.end())
            {
                m_workListDone.erase(it);
            }
        }
        void JDManagerAsyncWorker::clearDoneWork()
        {
            JDM_UNIQUE_LOCK_M(m_mutexInternal);
            m_workListDone.clear();
        }

        void JDManagerAsyncWorker::process()
        {
            JDM_UNIQUE_LOCK_M(m_mutexInternal);
            if (m_workList.size() == 0)
                return;
            m_cv.notify_all();
        }
        void JDManagerAsyncWorker::start()
        {
            if (m_thread)
                return;
            m_stopFlag.store(false);
            m_thread = new std::thread(&JDManagerAsyncWorker::threadLoop, this);
        }
        void JDManagerAsyncWorker::stop()
        {
            if (!m_thread)
                return;
            {
                JDM_UNIQUE_LOCK_M(m_mutexInternal);
                m_stopFlag.store(true);
                m_cv.notify_all();
            }
            m_thread->join();
            delete m_thread;
            m_thread = nullptr;
        }
        void JDManagerAsyncWorker::threadLoop()
        {
            JD_PROFILING_THREAD((m_manager.getDatabaseName()+"::"+ m_manager.getUser() + " JDManagerAsyncWorker").c_str());
            while (!m_stopFlag.load())
            {
                {
                    std::unique_lock<std::mutex> lock(m_mutexInternal);
                    JD_ASYNC_WORKER_PROFILING_BLOCK("JDManagerAsyncWorker::threadLoop::idle", JD_COLOR_STAGE_1);
                    bool stopFlag = m_stopFlag.load();
 
                    // Wait until we have work to do
                    m_cv.wait(lock);

                    if (stopFlag) {
                        break;
                    }
                }

                bool hasWork = true;
                {
                    JD_ASYNC_WORKER_PROFILING_BLOCK("JDManagerAsyncWorker::threadLoop::work", JD_COLOR_STAGE_1);
                    while (hasWork)
                    {
                        std::vector<JDManagerAysncWork*> workCPY;
                        {
                            JDM_UNIQUE_LOCK_M(m_mutexInternal);
                            workCPY = m_workList;
                            m_workList.clear();
                        }
                        processWork(workCPY);
                        {
                            JDM_UNIQUE_LOCK_M(m_mutexInternal);
                            hasWork = !m_workList.empty();
                        }
                    }
                }
            }
        }
        void JDManagerAsyncWorker::processWork(const std::vector<JDManagerAysncWork*>& workList)
        {
            JD_ASYNC_WORKER_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            for (auto& work : workList)
            {
                processWork(work);
            }
        }
        void JDManagerAsyncWorker::processWork(JDManagerAysncWork* work)
        {
            JD_ASYNC_WORKER_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
            work->process();
            {
                JDM_UNIQUE_LOCK_M(m_mutexInternal);
				m_workListDone.push_back(work);
            }
            m_manager.onAsyncWorkDone(work);
        }
    }
}