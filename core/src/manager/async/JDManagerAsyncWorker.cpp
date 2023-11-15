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

        void JDManagerAsyncWorker::addWork(std::shared_ptr<JDManagerAysncWork> work)
        {
            JD_ASYNC_WORKER_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
            JDM_UNIQUE_LOCK_M(m_workListMutex);
            m_workList.push_back(work);
        }
        bool JDManagerAsyncWorker::isWorkDone(std::shared_ptr<JDManagerAysncWork> work)
        {
            JD_ASYNC_WORKER_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
            JDM_UNIQUE_LOCK_M(m_mutexInternal);
            return std::find(m_workListDone.begin(), m_workListDone.end(), work) != m_workListDone.end();
        }
        void JDManagerAsyncWorker::removeDoneWork(std::shared_ptr<JDManagerAysncWork> work)
        {
            JD_ASYNC_WORKER_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
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
            {
                JDM_UNIQUE_LOCK_M(m_workListMutex);
                if (m_workList.size() == 0)
                    return;
            }
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

        WorkProgress JDManagerAsyncWorker::getWorkProgress() const
        {
            WorkProgress p;
            std::shared_ptr<JDManagerAysncWork> currentWork = m_currentWork.load();
            if (currentWork)
            {
				p = currentWork->m_progress;
			}
            return p;
        }
        bool JDManagerAsyncWorker::isBusy() const
        {
        	return m_busy.load();
        }
        WorkType JDManagerAsyncWorker::getCurrentWorkType() const
        {
            std::shared_ptr<JDManagerAysncWork> currentWork = m_currentWork.load();
            return currentWork->getWorkType();
        }


        void JDManagerAsyncWorker::threadLoop()
        {
            JD_PROFILING_THREAD((m_manager.getDatabaseName()+"::"+ m_manager.getUser() + " JDManagerAsyncWorker").c_str());
            while (!m_stopFlag.load())
            {
                m_busy.store(false);
                {
                    std::unique_lock<std::mutex> lock(m_mutexInternal);
                    JD_ASYNC_WORKER_PROFILING_BLOCK("JDManagerAsyncWorker::threadLoop::idle", JD_COLOR_STAGE_1);

                    // Wait until we have work to do
                    m_cv.wait(lock);

                    if (m_stopFlag.load()) {
                        break;
                    }
                }
                m_busy.store(true);
                bool hasWork = true;
                {
                    JD_ASYNC_WORKER_PROFILING_BLOCK("JDManagerAsyncWorker::threadLoop::work", JD_COLOR_STAGE_1);
                    while (hasWork)
                    {
                        std::vector<std::shared_ptr<JDManagerAysncWork>> workCPY;
                        {
                            JDM_UNIQUE_LOCK_M(m_workListMutex);
                            workCPY = m_workList;
                            m_workList.clear();
                        }
                        processWork(workCPY);
                        {
                            JDM_UNIQUE_LOCK_M(m_workListMutex);
                            hasWork = !m_workList.empty();
                        }
                    }
                }
            }
        }
        void JDManagerAsyncWorker::processWork(const std::vector<std::shared_ptr<JDManagerAysncWork>>& workList)
        {
            JD_ASYNC_WORKER_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            for (auto& work : workList)
            {
                processWork(work);
            }
        }
        void JDManagerAsyncWorker::processWork(std::shared_ptr<JDManagerAysncWork> work)
        {
            JD_ASYNC_WORKER_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
            m_currentWork.store(work);
            work->process();
            {
                JDM_UNIQUE_LOCK_M(m_mutexInternal);
				m_workListDone.push_back(work);
            }
            if (work->hasSucceeded())
            {
                work->m_progress.setProgress(1.0);
                work->m_progress.setComment("Done");
            }
            else
            {
                work->m_progress.setProgress(0.0);
                work->m_progress.setComment(work->getErrorMessage());
            }
            m_currentWork.store(nullptr);
            m_manager.onAsyncWorkDone(work);
        }
    }
}